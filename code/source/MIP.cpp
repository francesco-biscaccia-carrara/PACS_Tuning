#include "../include/MIP.hpp"

#define BOTH_BOUNDS 'B'
#define LW_BOUND 'L'
#define UP_BOUND 'U'
#define LE 'L'
#define EQ 'E'
#define GE 'G'

using MIPEx = MIPException::ExceptionType;

MIP::MIP(const std::string fileName) {
#if ACS_VERBOSE == DEBUG
	std::ostringstream oss;
	this->fileName = fileName;
	oss << this;
	this->id = oss.str();
#endif
	int status;
	env = CPXopenCPLEX(&status);
	model = CPXcreateprob(env, &status, "MIP");
	if (status)
		throw MIPException(MIPEx::ModelCreation, "Model not created!");

	status = CPXreadcopyprob(env, model, (INST_DIR + fileName + ".mps.gz").c_str(), NULL);

	if (status)
		throw MIPException(MIPEx::FileNotFound, "Failed to read the problem from file!\t" + std::to_string(status));

		// CPXsetdblparam(env, CPX_PARAM_EPGAP, MIP_GAP_TOL);
		// CPXsetdblparam(env, CPX_PARAM_EPAGAP, MIP_DUAL_PRIM_GAP_TOL);
#if ACS_VERBOSE == DEBUG
	CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
#endif
}

MIP::MIP(const MIP& otherMIP) {
#if ACS_VERBOSE == DEBUG
	std::ostringstream oss;
	this->fileName = otherMIP.fileName;
	oss << this;
	this->id = oss.str();
#endif
	int status;
	env = CPXopenCPLEX(&status);
	model = CPXcloneprob(env, otherMIP.model, &status);

	if (status)
		throw MIPException(MIPEx::ModelCreation, "Model not cloned!");

#if ACS_VERBOSE == DEBUG
	CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
#endif
}

MIP& MIP::setNumCores(const int numCores) {
	if (CPXsetintparam(env, CPX_PARAM_THREADS, numCores))
		throw MIPException(MIPEx::General, "Number of dedicated cores not changed!");
	return *this;
}

MIP& MIP::setNumSols(const int numSols) {
	if (CPXsetintparam(env, CPX_PARAM_INTSOLLIM, numSols))
		throw MIPException(MIPEx::General, "Number of max solutions not changed!");
	return *this;
}

size_t MIP::getNumNonZeros() {
	int nnz {(CPXgetnumnz(env, model)) };
	if (!nnz)
		throw MIPException(MIPEx::General, "Unable to get the number of nonzero elements!");
	return  static_cast<size_t>(nnz);
}

int MIP::solve(const double timeLimit, const double detTimeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	if (detTimeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_DETTILIM, detTimeLimit);

	if (int error{ CPXmipopt(env, model) })
		throw MIPException(MIPEx::MIP_OptimizationError, "CPLEX cannot solve this problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

MIP& MIP::addMIPStart(const std::vector<double>& MIPStart, bool CPLEXCheck) {
	size_t numCols{ getNumCols() };
	if (MIPStart.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong MIP start length");

	int start_index = 0;
	int effort_level = (CPLEXCheck) ? CPX_MIPSTART_CHECKFEAS : CPX_MIPSTART_NOCHECK;

	std::vector<int> indices(MIPStart.size(), 0);
	std::iota(indices.begin(), indices.end(), 0);

	if (int error{ CPXaddmipstarts(env, model, 1, MIPStart.size(), &start_index, indices.data(), MIPStart.data(), &effort_level, NULL) })
		throw MIPException(MIPEx::General, "Unable to set the MIP start!\t" + std::to_string(error));
	return *this;
}

double MIP::getObjValue() {
	double objValue;
	if (int error{ CPXgetobjval(env, model, &objValue) })
		throw MIPException(MIPEx::General, "Unable to obtain obj value!\t" + std::to_string(error));
	return objValue;
}

std::vector<double> MIP::getObjFunction() {
	size_t		numCols{ getNumCols() };
	double* objFun{ (double*)calloc(numCols, sizeof(double)) };
	if (CPXgetobj(env, model, objFun, 0, numCols - 1))
		throw MIPException(MIPEx::General, "Unable to get obj_function coefficients!");
	std::vector<double> obj(objFun, objFun + numCols);
	free(objFun);
	return obj;
}

MIP& MIP::setObjFunction(const std::vector<double>& newObj) {
	size_t numCols{ getNumCols() };
	if (newObj.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong new obj_function size");

	int* indices{ (int*)malloc(numCols * sizeof(int)) };
	for (size_t i{ 0 }; i < numCols; i++)
		indices[i] = i;
	if (CPXchgobj(env, model, numCols, indices, newObj.data()))
		throw MIPException(MIPEx::General, "obj_function not changed");
	free(indices);
	return *this;
}

std::vector<double> MIP::getSol() {
	size_t		numCols{ getNumCols() };
	double* xStar{ (double*)calloc(numCols, sizeof(double)) };
	if (int error{ CPXgetx(env, model, xStar, 0, numCols - 1) })
		throw MIPException(MIPEx::General, "Unable to obtain the solution! " + std::to_string(error) + " State: " + std::to_string(CPXgetstat(env, model)));
	std::vector<double> sol(xStar, xStar + numCols);
	free(xStar);
	return sol;
}

size_t MIP::getMIPNumVars() {
	int numCols{ CPXgetnumcols(env, model) };
	if(!numCols)
		throw MIPException(MIPEx::General, "Unable to get the number of MIP vars of the model!");
	return static_cast<size_t>(numCols);
}

size_t MIP::getNumCols() {
	int numCols{ CPXgetnumcols(env, model) };
	if(!numCols)
		throw MIPException(MIPEx::General, "Unable to get the number of cols of the model!");
	return static_cast<size_t>(numCols);
}

size_t MIP::getNumRows() {
	int numRows{ CPXgetnumrows(env, model) };
	if(!numRows)
		throw MIPException(MIPEx::General, "Unable to get the number of rows of the model!");
	return static_cast<size_t>(numRows);
}

MIP& MIP::addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name) {
		size_t numRow{ getNumRows() };

		if (newCol.size() != numRow)
			throw MIPException(MIPEx::InputSizeError, "Wrong new column size");

		char** cname{ (char**)calloc(1, sizeof(char*)) };
		cname[0] = strdup(name.c_str());

		int*	indices{ (int*)malloc(numRow * sizeof(int)) };
		double* values{ (double*)malloc(numRow * sizeof(double)) };
		int		start = 0, nnz = 0;
		for (size_t i = 0; i < numRow; i++) {
			if (newCol[i] != 0) {
				indices[nnz] = i;
				values[nnz] = newCol[i];
				nnz++;
			}
		}

		if (CPXaddcols(env, model, 1, nnz, &objCoef, &start, indices, values, &lb, &ub, &cname[0]))
			throw MIPException(MIPEx::General, "No column added!");
		free(cname);
		free(indices);
		free(values);
		return *this;
	}

MIP& MIP::addCol(const size_t index, const double value, const double objCoef, const double lb, const double ub, const std::string name) {
	if (index > getNumRows() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index addCol()!");

	char** cname{ (char**)calloc(1, sizeof(char*)) };
	cname[0] = strdup(name.c_str());

	int	   tmpIndex{ static_cast<int>(index) };
	double tmpValue{ value };
	int	   start{ 0 }, nnz{ 1 };

	if (CPXaddcols(env, model, 1, nnz, &objCoef, &start, &tmpIndex, &tmpValue, &lb, &ub, &cname[0]))
		throw MIPException(MIPEx::General, "No column added!");
	free(cname);
	return *this;
}

MIP& MIP::addRow(const std::vector<double>& newRow, const char sense, const double rhs) {
	size_t numCols{ getNumCols() };

	if (newRow.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong new row size");

	int*	indices{ (int*)malloc(numCols * sizeof(int)) };
	double* values{ (double*)malloc(numCols * sizeof(double)) };
	int		start = 0, nnz = 0;
	for (size_t i = 0; i < numCols; i++) {
		if (newRow[i] != 0) {
			indices[nnz] = i;
			values[nnz] = newRow[i];
			nnz++;
		}
	}

	if (CPXaddrows(env, model, 0, 1, nnz, &rhs, &sense, &start, indices, values, NULL, NULL))
		throw MIPException(MIPEx::General, "No row added!");
	free(indices);
	free(values);
	return *this;
}

MIP& MIP::removeRow(const int index) {
	if (index < 0 || static_cast<size_t>(index) > getNumRows() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index removeRow()!");

	if (CPXdelrows(env, model, index, index))
		throw MIPException(MIPEx::General, "Row not removed!");
	return *this;
}

MIP& MIP::removeCol(const int index) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index removeCol()!");

	if (CPXdelcols(env, model, index, index))
		throw MIPException(MIPEx::General, "Column not removed!");
	return *this;
}

VarBounds MIP::getVarBounds(const int index) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index getVarBounds()!");
	double lb = CPX_INFBOUND, ub = CPX_INFBOUND;
	if (CPXgetlb(env, model, &lb, index, index))
		throw MIPException(MIPEx::General, "Unable to get the var lower_bound!");

	if (CPXgetub(env, model, &ub, index, index))
		throw MIPException(MIPEx::General, "Unable to get the var upper_bound!");
	return VarBounds{ .lowerBound = lb, .upperBound = ub };
}

char MIP::getVarType(const int index) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index getVarType()!");

	char type;
	if (int error{ CPXgetctype(env, model, &type, index, index) })
		throw MIPException(MIPEx::General, "Unable to get var " + std::to_string(index) + "type!\t" + std::to_string(error));
	return type;
}

MIP& MIP::setVarValue(const int index, const double val) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");

	char bound{ BOTH_BOUNDS };
	if (CPXchgbds(env, model, 1, &index, &bound, &val))
		throw MIPException(MIPEx::General, "Unable to set the value to var " + std::to_string(val));
	return *this;
}

MIP& MIP::setVarLowerBound(const int index, const double newLB){
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");
	
	char bound{ LW_BOUND };
	if (CPXchgbds(env, model, 1, &index, &bound, &newLB))
		throw MIPException(MIPEx::General, "Unable to set the lower bound of the var  " + std::to_string(newLB));
	return *this;
}

MIP& MIP::setVarUpperBound(const int index, const double newUB){
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");
	
	char bound{ UP_BOUND };
	if (CPXchgbds(env, model, 1, &index, &bound, &newUB))
		throw MIPException(MIPEx::General, "Unable to set the upper bound of the var " + std::to_string(newUB));
	return *this;
}

MIP& MIP::setVarsValues(const std::vector<double>& values) {
	size_t numCols { getNumCols() };

	if (values.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong new values_array size!");
	for (size_t i{ 0 }; i < numCols; i++)
		if (values[i] < CPX_INFBOUND)
			setVarValue(i, values[i]);
	return *this;
}

double MIP::checkFeasibility(const std::vector<double>& sol) {
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");

	size_t	numRows = getNumRows();
	size_t	nzcnt = getNumNonZeros();

	int* rmatbeg = (int* ) malloc(numRows * sizeof(int));
    int* rmatind = (int* ) malloc(nzcnt * sizeof(int));
    double* rmatval = (double* ) malloc(nzcnt * sizeof(double));
	double* rhs = (double* )malloc(numRows * sizeof(double));
	char* sense = (char* )malloc(numRows * sizeof(char));

	if(CPXgetrhs(env, model, rhs, 0, numRows - 1))
		throw MIPException(MIPEx::General, "Error on retriving the RHS values");
	
	if(CPXgetsense(env, model, sense, 0, numRows - 1))
		throw MIPException(MIPEx::General, "Error on retriving the RHS values");
	

	int surplus, nnCPLEX; // Dummy values necessary for CPLXgetrows
	if(CPXgetrows(env, model, &nnCPLEX, rmatbeg, rmatind, rmatval, nzcnt, &surplus, 0, numRows - 1))
		throw MIPException(MIPEx::General, "Error on retriving the matrix rows");

	double maxViolation = 0.0;

	for (size_t i {0}; i < numRows; i++) {
        int start = rmatbeg[i];
        int end = (i == numRows - 1) ? nzcnt : rmatbeg[i + 1];

		double lhs = 0.0;
		for (int j = start; j < end; j++)
			lhs += sol[rmatind[j]] * rmatval[j];

		switch (sense[i]) {
			case LE:
				if(lhs > rhs[i] + maxViolation)
					maxViolation = lhs - rhs[i];
				break;

			case EQ :
				if(std::abs(lhs - rhs[i]) > maxViolation)
					maxViolation = std::abs(lhs - rhs[i]);
				break;

			case GE:
				if(lhs < rhs[i] - maxViolation)
					maxViolation = rhs[i] - lhs;
				break;

			default:
				throw MIPException(MIPEx::General, "Unknown type of sense");
				break;
		}
    }

	free(rmatbeg);
	free(rmatind);
	free(rmatval);
	free(rhs);
	free(sense);
	return maxViolation;
}

double MIP::checkIntegrality(const std::vector<double>& sol){
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");

	double maxIntViolation = 0.0;
	for (size_t i{ 0 }; i < sol.size(); i++) {
		char type = getVarType(i);
		if (type == CPX_BINARY || type == CPX_INTEGER){
/// FIXED: Bug #9fb83189145371b5c9acdfea1718509d9f332514 - Wrong computation of maxIntViolation
			double tmpIntVal = std::abs(sol[i] - std::round(sol[i]));
        	if (tmpIntVal > maxIntViolation) {
           	 	maxIntViolation = tmpIntVal;
        	}
		}
	}
	return maxIntViolation;
}

bool MIP::checkFeasibilityCPLEX(const std::vector<double>& sol){
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");

	// CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_Integrality, MIP_INT_TOL);
	// CPXsetdblparam(env, CPXPARAM_Simplex_Tolerances_Feasibility, MIP_SIMPLEX_FEAS_TOL);

	setVarsValues(sol);
	int status{ solve() };
	return (status == CPXMIP_OPTIMAL_TOL || status == CPXMIP_OPTIMAL);
}

double MIP::checkObjValue(const std::vector<double>& sol){
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");
	
	std::vector<double> objCoef = getObjFunction();
	return std::inner_product(objCoef.begin(), objCoef.end(), sol.begin(), 0.0);
}

MIP::~MIP() noexcept {
	CPXfreeprob(env, &model);
	CPXcloseCPLEX(&env);
}

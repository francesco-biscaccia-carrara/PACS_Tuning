#include "../include/MIP.hpp"

#define BOTH_BOUNDS 'B'
#define LW_BOUND 'L'
#define UP_BOUND 'U'

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

MIP& MIP::setCallbackFunction(CPXLONG contextMask, CPXCALLBACKFUNC* callback, void* data) {
	if (CPXcallbacksetfunc(env, model, contextMask, callback, data))
		throw MIPException(MIPEx::General, "Unable to set callback function!");
	return *this;
}

size_t MIP::getNumNonZeros() {
	size_t nnz{ static_cast<size_t>(CPXgetnumnz(env, model)) };
	if (!nnz)
		throw MIPException(MIPEx::General, "Unable to get the number of nonzero elements!");
	return nnz;
}

int MIP::solve(const double timeLimit, const double detTimeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	if (detTimeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_DETTILIM, detTimeLimit);

	if (int error{ CPXmipopt(env, model) })
		throw MIPException(MIPEx::MIPOptimizationError, "CPLEX cannot solve this problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

MIP& MIP::addMIPStart(const std::vector<double>& MIPStart, bool CPLEXCheck) {
	int numCols{ getNumCols() };
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
	int		numCols{ getNumCols() };
	double* objFun{ (double*)calloc(numCols, sizeof(double)) };
	if (CPXgetobj(env, model, objFun, 0, numCols - 1))
		throw MIPException(MIPEx::General, "Unable to get obj_function coefficients!");
	std::vector<double> obj(objFun, objFun + numCols);
	free(objFun);
	return obj;
}

MIP& MIP::setObjFunction(const std::vector<double>& newObj) {
	int numCols{ getNumCols() };
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
	int		numCols{ getNumCols() };
	double* xStar{ (double*)calloc(numCols, sizeof(double)) };
	if (int error{ CPXgetx(env, model, xStar, 0, numCols - 1) })
		throw MIPException(MIPEx::General, "Unable to obtain the solution! " + std::to_string(error) + " State: " + std::to_string(CPXgetstat(env, model)));
	std::vector<double> sol(xStar, xStar + numCols);
	free(xStar);
	return sol;
}

MIP& MIP::addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name) {
	int numRow{ getNumRows() };

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
	if (index < 0 || index > getNumRows() - 1)
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
	int numCols{ getNumCols() };

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
	if (index < 0 || index > getNumRows() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index removeRow()!");

	if (CPXdelrows(env, model, index, index))
		throw MIPException(MIPEx::General, "Row not removed!");
	return *this;
}

MIP& MIP::removeCol(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index removeCol()!");

	if (CPXdelcols(env, model, index, index))
		throw MIPException(MIPEx::General, "Column not removed!");
	return *this;
}

VarBounds MIP::getVarBounds(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index getVarBounds()!");
	double lb = CPX_INFBOUND, ub = CPX_INFBOUND;
	if (CPXgetlb(env, model, &lb, index, index))
		throw MIPException(MIPEx::General, "Unable to get the var lower_bound!");

	if (CPXgetub(env, model, &ub, index, index))
		throw MIPException(MIPEx::General, "Unable to get the var upper_bound!");
	return VarBounds{ .lowerBound = lb, .upperBound = ub };
}

MIP& MIP::setVarValue(const int index, const double val) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");

	char bound{ BOTH_BOUNDS };
	if (CPXchgbds(env, model, 1, &index, &bound, &val))
		throw MIPException(MIPEx::General, "Unable to set the value to var " + std::to_string(val));
	return *this;
}


MIP& MIP::setVarLowerBound(const int index, const double newLB){
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");
	
	char bound{ LW_BOUND };
	if (CPXchgbds(env, model, 1, &index, &bound, &newLB))
		throw MIPException(MIPEx::General, "Unable to set the lower bound of the var  " + std::to_string(newLB));
	return *this;
}

MIP& MIP::setVarUpperBound(const int index, const double newUB){
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValue()!");
	
	char bound{ UP_BOUND };
	if (CPXchgbds(env, model, 1, &index, &bound, &newUB))
		throw MIPException(MIPEx::General, "Unable to set the upper bound of the var " + std::to_string(newUB));
	return *this;
}

MIP& MIP::setVarsValues(const std::vector<double>& values) {
	int numCols{ getNumCols() };

	if (values.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong new values_array size!");
	for (size_t i{ 0 }; i < numCols; i++)
		if (values[i] < CPX_INFBOUND)
			setVarValue(i, values[i]);
	return *this;
}

bool MIP::checkFeasibility(const std::vector<double>& sol) {
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");

	// CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_Integrality, MIP_INT_TOL);
	// CPXsetdblparam(env, CPXPARAM_Simplex_Tolerances_Feasibility, MIP_SIMPLEX_FEAS_TOL);

	setVarsValues(sol);
	int status{ solve() };
	return (status == CPXMIP_OPTIMAL_TOL || status == CPXMIP_OPTIMAL);
}

MIP::~MIP() noexcept {
	CPXfreeprob(env, &model);
	CPXcloseCPLEX(&env);
}

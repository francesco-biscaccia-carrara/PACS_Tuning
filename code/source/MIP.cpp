#include "../include/MIP.hpp"

#define BOTH_BOUNDS 'B'
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

	status = CPXreadcopyprob(env, model, (INST_DIR + fileName + ".mps").c_str(), "MPS");
	if (status)
		throw MIPException(MIPEx::FileNotFound, "Failed to read the problem from file!\t" + std::to_string(status));

	CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_MIPGap, MIP_GAP_TOL);
	CPXsetdblparam(env, CPX_PARAM_EPAGAP, MIP_DUAL_PRIM_GAP_TOL);
#if ACS_VERBOSE == DEBUG
	CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
#endif

	restoreVarType.reserve(getNumCols());
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

	CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_MIPGap, MIP_GAP_TOL);
	CPXsetdblparam(env, CPX_PARAM_EPAGAP, MIP_DUAL_PRIM_GAP_TOL);
#if ACS_VERBOSE == DEBUG
	CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
#endif

	restoreVarType.reserve(getNumCols());
}


MIP& MIP::setNumCores(const int numCores) {
	if (CPXsetintparam(env, CPX_PARAM_THREADS, numCores))
		throw MIPException(MIPEx::General, "Number of dedicated cores not changed!");
	return *this;
}

MIP& MIP::setTerminate(int* p){
	if(CPXsetterminate(env,p))
		throw MIPException(MIPEx::General, "Unable to set terminate condition!");
	return *this;
}
int MIP::solve(const double timeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	for (size_t i{ 0 }; i < restoreVarType.size(); i++)
		changeVarType(i, restoreVarType[i]);
	changeProbType(CPXPROB_MILP);

	if (int error{ CPXmipopt(env, model) })
		throw MIPException(MIPEx::MIPOptimizationError, "CPLEX cannot solve this problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

int MIP::solveRelaxation(const double timeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	changeProbType(CPXPROB_MILP); // Necessary to get vars type

	for (size_t i{ 0 }; i < getNumCols(); i++) {
		char type = getVarType(i);
		if (type == CPX_BINARY || type == CPX_INTEGER)
			restoreVarType[i] = type;
	}
	changeProbType(CPXPROB_LP);

	if (int error{ CPXlpopt(env, model) })
		throw MIPException(MIPEx::LPOptimizationError, "CPLEX cannot solve the relaxed problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
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
	if (CPXgetx(env, model, xStar, 0, numCols - 1))
		throw MIPException(MIPEx::General, "Unable to obtain the solution!");
	std::vector<double> sol(xStar, xStar + numCols);
	free(xStar);
	return sol;
}

MIP& MIP::addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name) {
	int numRow{ getNumRows() };

	if (newCol.size() != numRow)
		throw MIPException(MIPEx::InputSizeError, "Wrong new column size");

	char** cname{ (char**)calloc(1, sizeof(char*)) };
	char   colName[name.length()];
	strcpy(colName, name.c_str());
	cname[0] = colName;
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
	return VarBounds{ .lowerBound{ lb }, .upperBound{ ub } };
}

char MIP::getVarType(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index getVarType()!");

	char type;
	if (int error{ CPXgetctype(env, model, &type, index, index) })
		throw MIPException(MIPEx::General, "Unable to get var " + std::to_string(index) + "type!\t" + std::to_string(error));
	return type;
}

MIP& MIP::changeVarType(const int index, const char type) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index changeVarType()!");

	if (CPXchgctype(env, model, 1, &index, &type))
		throw MIPException(MIPEx::General, "Type of var" + std::to_string(index) + "not changed!");
	return *this;
}

MIP& MIP::setVarValues(const int index, const double val) {
	if (index < 0 || index > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index setVarValues()!");

	char bound{ BOTH_BOUNDS };
	CPXchgbds(env, model, 1, &index, &bound, &val);
	return *this;
}

MIP& MIP::setVarsValues(const std::vector<double>& values) {
	int numCols{ getNumCols() };

	if (values.size() != numCols)
		throw MIPException(MIPEx::InputSizeError, "Wrong new values_array size!");
	for (size_t i{ 0 }; i < numCols; i++)
		if (values[i] < CPX_INFBOUND)
			setVarValues(i, values[i]);
	return *this;
}

bool MIP::checkFeasibility(const std::vector<double>& sol) {
	if (sol.size() != getNumCols())
		throw MIPException(MIPEx::InputSizeError, "Wrong solution size!");

	setVarsValues(sol);
	int status{ solve(CPX_INFBOUND) };
	return status == CPXMIP_OPTIMAL_TOL || status == CPXMIP_OPTIMAL;
}

MIP::~MIP() {
	CPXfreeprob(env, &model);
	CPXcloseCPLEX(&env);
}

MIP& MIP::changeProbType(const int type) {
	if (CPXchgprobtype(env, model, type))
		throw MIPException(MIPEx::General, "Problem type not changed!");

	return *this;
}

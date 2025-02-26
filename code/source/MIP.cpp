#include "../include/MIP.hpp"

#define BOTH_BOUNDS 'B'

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
		Logger::print(Logger::LogLevel::ERROR, "Model not created!");

	status = CPXreadcopyprob(env, model, (INST_DIR + fileName + ".mps").c_str(), "MPS");
	if (status)
		Logger::print(Logger::LogLevel::ERROR, "Failed to read the problem from file! %d", status);

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
		Logger::print(Logger::LogLevel::ERROR, "Model not cloned!");

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
		Logger::print(Logger::LogLevel::ERROR, "CPX_PARAM_THREADS not cahanged!");
	return *this;
}

int MIP::solve(const double timeLimit) {

	if (timeLimit < EPSILON)
		Logger::print(Logger::LogLevel::ERROR, "Time-limit (%10.4f) is too short!", timeLimit);

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	for (size_t i{ 0 }; i < restoreVarType.size(); i++)
		changeVarType(i, restoreVarType[i]);
	changeProbType(CPXPROB_MILP);

	if (int error{ CPXmipopt(env, model) })
		Logger::print(Logger::LogLevel::ERROR, "CPLEX cannot solve this problem! %d", error);

	return CPXgetstat(env, model);
}

int MIP::solveRelaxation(const double timeLimit) {

	if (timeLimit < EPSILON)
		Logger::print(Logger::LogLevel::ERROR, "Time-limit (%10.4f) is too short!", timeLimit);

	if (timeLimit < CPX_INFBOUND)
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	changeProbType(CPXPROB_MILP); // Necessary to get vars type

	for (size_t i{ 0 }; i < getNumCols(); i++) {
		char type = getVarType(i);
		if (type == CPX_BINARY || type == CPX_INTEGER)
			restoreVarType[i] = type;
	}
	changeProbType(CPXPROB_LP);

	if (int error{ CPXlpopt(env, model) })
		Logger::print(Logger::LogLevel::ERROR, "CPLEX cannot solve the problem relaxation! %d", error);

	return CPXgetstat(env, model);
}

double MIP::getObjValue() {
	double objValue;
	if (int error{ CPXgetobjval(env, model, &objValue) })
		Logger::print(Logger::LogLevel::ERROR, "Unable to obtain obj value! %d", error);
	return objValue;
}

std::vector<double> MIP::getObjFunction() {
	int		numCols{ getNumCols() };
	double* objFun{ (double*)calloc(numCols, sizeof(double)) };
	if (CPXgetobj(env, model, objFun, 0, numCols - 1))
		Logger::print(Logger::LogLevel::ERROR, "Unable to get obj. coefficients!");
	std::vector<double> obj(objFun, objFun + numCols);
	free(objFun);
	return obj;
}

MIP& MIP::setObjFunction(const std::vector<double>& newObj) {
	int numCols{ getNumCols() };
	if (newObj.size() != numCols)
		Logger::print(Logger::LogLevel::ERROR, "No suitable obj_function coefficients"); // TODO: Assert type

	int* indices{ (int*)malloc(numCols * sizeof(int)) };
	for (size_t i{ 0 }; i < numCols; i++)
		indices[i] = i;
	if (CPXchgobj(env, model, numCols, indices, &newObj[0]))
		Logger::print(Logger::LogLevel::ERROR, "obj_function not changed");
	free(indices);
	return *this;
}

std::vector<double> MIP::getSol() {
	int		numCols{ getNumCols() };
	double* xStar{ (double*)calloc(numCols, sizeof(double)) };
	if (CPXgetx(env, model, xStar, 0, numCols - 1))
		Logger::print(Logger::LogLevel::ERROR, "Unable to obtain the solution!");
	std::vector<double> sol(xStar, xStar + numCols);
	free(xStar);
	return sol;
}

MIP& MIP::addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name) {
	int numRow{ getNumRows() };

	if (newCol.size() != numRow)
		Logger::print(Logger::LogLevel::ERROR, "Wrong column size!"); // TODO: Assert type

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
		Logger::print(Logger::LogLevel::ERROR, "No Column added!");
	free(cname);
	free(indices);
	free(values);
	return *this;
}

MIP& MIP::addRow(const std::vector<double>& newRow, const char sense, const double rhs) {
	int numCols{ getNumCols() };

	if (newRow.size() != numCols)
		Logger::print(Logger::LogLevel::ERROR, "Wrong row size!"); // TODO: Assert type exe

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
		Logger::print(Logger::LogLevel::ERROR, "No Column added!");
	free(indices);
	free(values);
	return *this;
}

MIP& MIP::removeRow(const int index) {
	if (index < 0 || index > getNumRows() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index removeRow()!");

	if (CPXdelrows(env, model, index, index))
		Logger::print(Logger::LogLevel::ERROR, "Row not removed!");
	return *this;
}

MIP& MIP::removeCol(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index removeCol()!");

	if (CPXdelcols(env, model, index, index))
		Logger::print(Logger::LogLevel::ERROR, "Row not removed!");
	return *this;
}

VarBounds MIP::getVarBounds(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index getVarBounds()!");

	double lb = CPX_INFBOUND, ub = CPX_INFBOUND;
	if (CPXgetlb(env, model, &lb, index, index))
		Logger::print(Logger::LogLevel::ERROR, "Unable to get the var lower_bound!");
	if (CPXgetub(env, model, &ub, index, index))
		Logger::print(Logger::LogLevel::ERROR, "Unable to get the var upper_bound!");
	return VarBounds{ .lowerBound{ lb }, .upperBound{ ub } };
}

char MIP::getVarType(const int index) {
	if (index < 0 || index > getNumCols() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index getVarType()!");

	char type;
	if (int error{ CPXgetctype(env, model, &type, index, index) })
		Logger::print(Logger::LogLevel::ERROR, "Unable to get var %d type! %d", index, error);
	return type;
}

MIP& MIP::changeVarType(const int index, const char type) {
	if (index < 0 || index > getNumCols() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index changeVarType()!");

	if (CPXchgctype(env, model, 1, &index, &type))
		Logger::print(Logger::LogLevel::ERROR, "Type of var %d not changed!", index);
	return *this;
}

MIP& MIP::setVarValues(const int index, const double val) {
	if (index < 0 || index > getNumCols() - 1)
		Logger::print(Logger::LogLevel::ERROR, "Wrong index in setVarValues()!");

	char bound{ BOTH_BOUNDS };
	CPXchgbds(env, model, 1, &index, &bound, &val);
	return *this;
}

MIP& MIP::setVarsValues(const std::vector<double>& values) {
	int numCols{ getNumCols() };

	if (values.size() != numCols)
		Logger::print(Logger::LogLevel::ERROR, "Wrong values size!");
	for (size_t i{ 0 }; i < numCols; i++)
		if (values[i] < CPX_INFBOUND / 2)
			setVarValues(i, values[i]);
	return *this;
}

MIP::~MIP() {
	CPXfreeprob(env, &model);
	CPXcloseCPLEX(&env);
}

MIP& MIP::changeProbType(const int type) {
	if (CPXchgprobtype(env, model, type))
		Logger::print(Logger::LogLevel::ERROR, "Problem type not changed");
	return *this;
}

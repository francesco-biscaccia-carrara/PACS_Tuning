#include "../include/RlxFMIP.hpp"
using MIPEx = MIPException::ExceptionType;

RlxFMIP::RlxFMIP(std::string fileName) : FMIP(fileName) {
	restoreVarType.reserve(getNumCols());
#if ACS_VERBOSE == DEBUG
	this->fileName += "_RlxFMIP";
#endif
}

int RlxFMIP::solve(const double timeLimit, const double detTimeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	if (detTimeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_DETTILIM, detTimeLimit);

	for (size_t i{ 0 }; i < restoreVarType.size(); i++)
		changeVarType(i, restoreVarType[i]);
	changeProbType(CPXPROB_MILP);

	if (int error{ CPXmipopt(env, model) })
		throw MIPException(MIPEx::MIP_OptimizationError, "CPLEX cannot solve this problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

int RlxFMIP::solveRelaxation(const double timeLimit, const double detTimeLimit) {

	if (timeLimit < EPSILON)
		throw MIPException(MIPEx::WrongTimeLimit, "Time-limit too short!\t" + std::to_string(timeLimit));

	if (timeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_TILIM, timeLimit);

	if (detTimeLimit < CPX_INFBOUND) [[likely]]
		CPXsetdblparam(env, CPX_PARAM_DETTILIM, detTimeLimit);

	changeProbType(CPXPROB_MILP); // Necessary to get vars type

	for (size_t i{ 0 }; i < getNumCols(); i++) {
		char type = getVarType(i);
		if (type == CPX_BINARY || type == CPX_INTEGER)
			restoreVarType[i] = type;
	}
	changeProbType(CPXPROB_LP);

	if (int error{ CPXlpopt(env, model) })
		throw MIPException(MIPEx::LP_OptimizationError, "CPLEX cannot solve the relaxed problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

char RlxFMIP::getVarType(const int index) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index getVarType()!");

	char type;
	if (int error{ CPXgetctype(env, model, &type, index, index) })
		throw MIPException(MIPEx::General, "Unable to get var " + std::to_string(index) + "type!\t" + std::to_string(error));
	return type;
}

RlxFMIP& RlxFMIP::changeVarType(const int index, const char type) {
	if (index < 0 || static_cast<size_t>(index) > getNumCols() - 1)
		throw MIPException(MIPEx::OutOfBound, "Wrong index changeVarType()!");

	if (CPXchgctype(env, model, 1, &index, &type))
		throw MIPException(MIPEx::SetFunction, "Type of var" + std::to_string(index) + "not changed!");
	return *this;
}

void RlxFMIP::changeProbType(const int type) {
	if (CPXchgprobtype(env, model, type))
		throw MIPException(MIPEx::SetFunction, "Problem type not changed!");
}
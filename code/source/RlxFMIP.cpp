#include "../include/RlxFMIP.hpp"
using MIPEx = MIPException::ExceptionType;

RlxFMIP::RlxFMIP(std::string fileName): FMIP(fileName){
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
		throw MIPException(MIPEx::MIPOptimizationError, "CPLEX cannot solve this problem!\t" + std::to_string(error));

	return CPXgetstat(env, model);
}

int RlxFMIP::solveRelaxation(const double timeLimit) {

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

void RlxFMIP::changeProbType(const int type) {
	if (CPXchgprobtype(env, model, type))
		throw MIPException(MIPEx::General, "Problem type not changed!");
}
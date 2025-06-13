#include "../include/MTContext.hpp"

MTContext::MTContext(size_t subMIPNum, unsigned long long intialSeed) : numMIPs{ subMIPNum } {

	bestACSIncumbent = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
	tmpSolutions = std::vector<Solution>();
	threads = std::vector<std::thread>();
	A_RhoChanges = 0;

	threads.reserve(numMIPs);
	tmpSolutions.reserve(numMIPs);
	rndGens.reserve(numMIPs);

	for (size_t i{ 0 }; i < numMIPs; i++) {
		rndGens.emplace_back(intialSeed + (i + 1));
		tmpSolutions.push_back({ .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND });
	}

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("MT Context: Initialized -- Num schedulable jobs: %d", numMIPs);
#endif
}

void MTContext::setBestACSIncumbent(Solution& sol) {
	std::lock_guard<std::mutex> lock(MTContextMTX);

	/// FIXED: Bug #9d6c973e040c2a8da57051d05e46d92c6e366c45 -- Inconsistency between actual and saved objective values due to incorrect slack sum comparison
	if ( (sol.slackSum < bestACSIncumbent.slackSum) || (std::abs(sol.slackSum) < EPSILON && sol.oMIPCost < bestACSIncumbent.oMIPCost) ) {		
		bestACSIncumbent = { .sol = sol.sol, .slackSum = sol.slackSum, .oMIPCost = sol.oMIPCost };
		A_RhoChanges = numMIPs;

		if (bestACSIncumbent.oMIPCost < CPX_INFBOUND && bestACSIncumbent.slackSum <= EPSILON)
			PRINT_BEST("New MIP Incumbent found %12.2f\t[*]", bestACSIncumbent.oMIPCost);

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("New ACS Incumbent found %12.2f|%-10.2f\t[*]", bestACSIncumbent.oMIPCost, bestACSIncumbent.slackSum);
#endif
	}
}

MTContext& MTContext::broadcastSol(Solution& tmpSol) {
	waitAllJobs();

	for (size_t i{ 0 }; i < numMIPs; i++) {
		threads.emplace_back(&MTContext::setTmpSolution, this, i, std::ref(tmpSol));
	}

	waitAllJobs();
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("MT Context: Broadcasting main sol to all threads");
#endif
	return *this;
}

void MTContext::waitAllJobs() {
	for (auto& thread : threads) {
		if (thread.joinable()) {
			thread.join();
		}
	}
	threads.clear();
	A_RhoChanges = 0;
}

MTContext& MTContext::parallelFMIPOptimization(Args& CLIArgs) {
	waitAllJobs();

	for (size_t i{ 0 }; i < CLIArgs.numsubMIPs; i++) {
		threads.emplace_back(&MTContext::FMIPInstanceJob, this, i, std::ref(CLIArgs));
	}

	waitAllJobs();
	return *this;
}

MTContext& MTContext::parallelOMIPOptimization(Args& CLIArgs) {
	waitAllJobs();

	for (size_t i{ 0 }; i < CLIArgs.numsubMIPs; i++) {
		threads.emplace_back(&MTContext::OMIPInstanceJob, this, i, std::ref(CLIArgs));
	}

	waitAllJobs();
	return *this;
}


MTContext::~MTContext() {
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("MT Context: Closed");
#endif
}

void MTContext::FMIPInstanceJob(const size_t thID, Args& CLIArgs) {

	FMIP fMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		fMIP.addMIPStart(bestACSIncumbent.sol);
		FixPolicy::fixSlackUpperBoundMT(thID, "FMIP", fMIP, bestACSIncumbent.sol);
	}
	fMIP.setNumCores(CPLEX_CORE);

	FixPolicy::randomRhoFixMT(thID, "FMIP", fMIP, tmpSolutions[thID].sol, CLIArgs.rho, rndGens[thID]);

	/// FIXED: Bug #e15760bcfd3dcca51cf9ea23f70072dd6cb2ac14 — Resolved MIPException::WrongTimeLimit triggered by a negligible time limit.
	if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("TIME_LIMIT REACHED");
#endif
		return;
	}

	int solveCode{ fMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), DET_TL(fMIP.getNumNonZeros())) };
	if (MIP::isINForUNBD(solveCode)) {
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Proc: %3d [FMIP] - Aborted: Infeasible with given TL", thID);
#endif
		return;
	}

	tmpSolutions[thID].sol = fMIP.getSol();
	tmpSolutions[thID].slackSum = fMIP.getObjValue();
	/// FIXED: Bug #9d6c973e040c2a8da57051d05e46d92c6e366c45 -- Inconsistency between actual and saved objective values due to incorrect slack sum comparison
	tmpSolutions[thID].oMIPCost = fMIP.getOMIPCost(tmpSolutions[thID].sol);

	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID, tmpSolutions[thID].slackSum);
	setBestACSIncumbent(tmpSolutions[thID]);

	FixPolicy::dynamicAdjustRhoMT(thID, "FMIP", solveCode, numMIPs, CLIArgs.rho, A_RhoChanges);
}

#pragma region MTContextPrivateSec

void MTContext::OMIPInstanceJob(const size_t thID, Args& CLIArgs) {

	OMIP oMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		oMIP.addMIPStart(bestACSIncumbent.sol);
		FixPolicy::fixSlackUpperBoundMT(thID, "OMIP", oMIP, bestACSIncumbent.sol);
	}
	oMIP.setNumCores(CPLEX_CORE);

	FixPolicy::randomRhoFixMT(thID, "OMIP", oMIP, tmpSolutions[thID].sol, CLIArgs.rho, rndGens[thID]);

	/// FIXED: Bug #e15760bcfd3dcca51cf9ea23f70072dd6cb2ac14 — Resolved MIPException::WrongTimeLimit triggered by a negligible time limit.
	if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("TIME_LIMIT REACHED");
#endif
		return;
	}

	int solveCode{ oMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), DET_TL(oMIP.getNumNonZeros())) };
	if (MIP::isINForUNBD(solveCode)) {
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Proc: %3d [OMIP] - Aborted: Infeasible with given TL", thID);
#endif
		return;
	}

	tmpSolutions[thID].sol = oMIP.getSol();
	tmpSolutions[thID].slackSum = oMIP.getSlackSum();
	tmpSolutions[thID].oMIPCost = oMIP.getObjValue();

	PRINT_OUT("Proc: %3d - OptMIP Objective: %20.2f|%-10.2f", thID, tmpSolutions[thID].oMIPCost, tmpSolutions[thID].slackSum);
	setBestACSIncumbent(tmpSolutions[thID]);

	FixPolicy::dynamicAdjustRhoMT(thID, "OMIP", solveCode, numMIPs, CLIArgs.rho, A_RhoChanges);
}

#pragma endregion
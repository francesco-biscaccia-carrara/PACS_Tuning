#include "../include/MTContext.hpp"

MTContext::MTContext(size_t subMIPNum, unsigned long long intialSeed) : numMIPs{ subMIPNum } {

	bestACSIncumbent = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
	tmpSolutions = std::vector<Solution>();
	threads = std::vector<std::thread>();

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
	if((sol.oMIPCost < bestACSIncumbent.oMIPCost && abs(bestACSIncumbent.slackSum - sol.slackSum) < EPSILON) || sol.slackSum< bestACSIncumbent.slackSum){

		std::lock_guard<std::mutex> lock(updateSolMTX);
		bestACSIncumbent = { .sol = sol.sol, .slackSum = sol.slackSum, .oMIPCost = sol.oMIPCost };
	#if ACS_VERBOSE >= VERBOSE
		if(bestACSIncumbent.slackSum < EPSILON && bestACSIncumbent.oMIPCost< CPX_INFBOUND)
			PRINT_BEST("New MIP Incumbent found %12.2f\t[*]", bestACSIncumbent.oMIPCost);
		else
			PRINT_INFO("New ACS Incumbent found %12.2f|%-10.2f\t[*]", bestACSIncumbent.oMIPCost,bestACSIncumbent.slackSum);
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
}

MTContext& MTContext::parallelFMIPOptimization(double remTime, Args CLIArgs) {
	waitAllJobs();

	for (size_t i{ 0 }; i < CLIArgs.numsubMIPs; i++) {
		threads.emplace_back(&MTContext::FMIPInstanceJob, this, i, remTime, CLIArgs);
	}

	waitAllJobs();
	return *this;
}

MTContext& MTContext::parallelOMIPOptimization(double remTime, Args CLIArgs, double slackSumUB) {
	waitAllJobs();

	for (size_t i{ 0 }; i < CLIArgs.numsubMIPs; i++) {
		threads.emplace_back(&MTContext::OMIPInstanceJob, this, i, remTime, CLIArgs, slackSumUB);
	}

	waitAllJobs();
	return *this;
}

MTContext::~MTContext() {
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("MT Context: Closed");
#endif
}

void MTContext::FMIPInstanceJob(size_t thID, double remTime, Args CLIArgs) {

	FMIP fMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		fMIP.addMIPStart(bestACSIncumbent.sol);
	}
	fMIP.setNumCores(CPLEX_CORE);

	FixPolicy::randomRhoFix(tmpSolutions[thID].sol, fMIP, thID, CLIArgs.rho, "FMIP", rndGens[thID]);

	int solveCode{ fMIP.solve(remTime, DET_TL(fMIP.getNumNonZeros())) };
	if (solveCode == CPXMIP_TIME_LIM_INFEAS || solveCode == CPXMIP_DETTIME_LIM_INFEAS || solveCode == CPXMIP_INFEASIBLE){
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Proc: %3d [FMIP] - Aborted: Infeasible with given TL", thID);
#endif
		return;
	}

	tmpSolutions[thID].sol = fMIP.getSol();
	tmpSolutions[thID].slackSum = fMIP.getObjValue();

	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID, tmpSolutions[thID].slackSum);
	setBestACSIncumbent(tmpSolutions[thID]);
}

void MTContext::OMIPInstanceJob(size_t thID, double remTime, Args CLIArgs, double slackSumUB) {

	OMIP oMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		oMIP.addMIPStart(bestACSIncumbent.sol);
	}
	oMIP.setNumCores(CPLEX_CORE);
	oMIP.updateBudgetConstr(slackSumUB);

	FixPolicy::randomRhoFix(tmpSolutions[thID].sol, oMIP, thID, CLIArgs.rho, "OMIP", rndGens[thID]);

	int solveCode{ oMIP.solve(remTime, DET_TL(oMIP.getNumNonZeros())) };

	if (solveCode == CPXMIP_TIME_LIM_INFEAS || solveCode == CPXMIP_DETTIME_LIM_INFEAS || solveCode == CPXMIP_INFEASIBLE) {
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
}

#include "../include/MTContext.hpp"

MTContext::MTContext(size_t subMIPNum, unsigned long long intialSeed) : numMIPs{ subMIPNum }, bestIncumbent{ .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND } {

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

MTContext& MTContext::setBestIncumbent(Solution sol) {
	if (abs(sol.slackSum) > EPSILON) {
		return *this;
	}

	if (sol.oMIPCost >= bestIncumbent.oMIPCost) {
		return *this;
	}

	std::lock_guard<std::mutex> lock(updateSolMTX);
	bestIncumbent = { .sol = sol.sol, .slackSum = sol.slackSum, .oMIPCost = sol.oMIPCost };

	PRINT_BEST("New incumbent found %12.2f|%-10.2f\t [*]", bestIncumbent.oMIPCost, bestIncumbent.slackSum);

	return *this;
}

MTContext& MTContext::setTmpSolution(int index, Solution& tmpSol) {
	tmpSolutions[index] = tmpSol;
	return *this;
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
	fMIP.setNumCores(CPLEX_CORE);

	std::vector<size_t> varsToFix = randomRhoFix(tmpSolutions[thID].sol.size(), thID, CLIArgs.rho, "FMIP", rndGens[thID]);

	for (auto i : varsToFix) {
		fMIP.setVarValue(i, tmpSolutions[thID].sol[i]);
	}

	int solveCode{ fMIP.solve(remTime, CLIArgs.LNSDtimeLimit) };
	if (solveCode == CPXMIP_TIME_LIM_INFEAS)
		return;

	tmpSolutions[thID].sol = fMIP.getSol();
	tmpSolutions[thID].slackSum = fMIP.getObjValue();
	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID, tmpSolutions[thID].slackSum);
}

void MTContext::OMIPInstanceJob(size_t thID, double remTime, Args CLIArgs, double slackSumUB) {

	OMIP oMIP{ CLIArgs.fileName };
	oMIP.setNumCores(CPLEX_CORE);

	std::vector<size_t> varsToFix = randomRhoFix(tmpSolutions[thID].sol.size(), thID, CLIArgs.rho, "FMIP", rndGens[thID]);
	for (auto i : varsToFix) {
		oMIP.setVarValue(i, tmpSolutions[thID].sol[i]);
	}

	oMIP.updateBudgetConstr(slackSumUB);
	if (bestIncumbent.slackSum < EPSILON) {
		std::vector<double> MIPStart(bestIncumbent.sol);
		MIPStart.resize(oMIP.getNumCols(), 0.0);
		oMIP.addMIPStart(MIPStart);
	}

	int solveCode{ oMIP.solve(remTime, CLIArgs.LNSDtimeLimit) };

	if (solveCode == CPXMIP_TIME_LIM_INFEAS)
		return;

	tmpSolutions[thID].sol = oMIP.getSol();
	tmpSolutions[thID].slackSum = oMIP.getSlackSum();
	tmpSolutions[thID].oMIPCost = oMIP.getObjValue();

	PRINT_OUT("Proc: %3d - OptMIP Objective: %20.2f", thID, tmpSolutions[thID].oMIPCost);
	setBestIncumbent(tmpSolutions[thID]);
}

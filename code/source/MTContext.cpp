#include "../include/MTContext.hpp"

// FIXME: v0.0.9 - remove it

// #pragma region CPLEX_CALLBACK 

// std::mutex callbackMTX;

// int updatePoolsubMIP(CPXCALLBACKCONTEXTptr context, void* inc) {
// 	Solution* incSol = (Solution*)inc;

// 	std::vector<double> candidateFound(incSol->sol.size(), CPX_INFBOUND);
// 	double				value = 0.0;
// 	if (CPXcallbackgetcandidatepoint(context, candidateFound.data(), 0, incSol->sol.size() - 1, &value)) {
// 		PRINT_ERR("No candidate sol found!");
// 		return 0;
// 	}

// 	std::lock_guard<std::mutex> lock(callbackMTX);
// 	if(abs(incSol->slackSum) > EPSILON){
// 		if (incSol->slackSum > value) {
// 			incSol->sol = candidateFound;
// 			incSol->slackSum = value;
	
// #if ACS_VERBOSE >= VERBOSE
// 			PRINT_INFO("MT Context: [FMIP] - Candidate sol shared to pool! FMIP-Cost: %10.4f", incSol->slackSum);
// #endif
// 		}
// 	}else{
// 		if (incSol->oMIPCost > value) {
// 			incSol->sol = candidateFound;
// 			incSol->oMIPCost = value;
	
// 	#if ACS_VERBOSE >= VERBOSE
// 			PRINT_INFO("MT Context: [OMIP] - Candidate sol shared to pool! OMIP-Cost: %10.4f", incSol->oMIPCost);
// 	#endif
// 		}

// 	}
	
// 	return 0;
// }

// int updateLocalSolsubMIP(CPXCALLBACKCONTEXTptr context, void* inc) {
// 	Solution* incSol = (Solution*)inc;

// 	int				 cnt = incSol->sol.size();
// 	std::vector<int> indices(cnt, 0);
// 	std::iota(indices.begin(), indices.end(), 0);

// 	if(abs(incSol->slackSum) > EPSILON){
// 		if (int status = CPXcallbackpostheursoln(context, cnt, indices.data(), incSol->sol.data(), incSol->slackSum, CPXCALLBACKSOLUTION_NOCHECK)) {
// 			PRINT_ERR("[FMIP] - Unable to set the heuristic - Code: %d", status);
// 			return 0;
// 		}
// 	}else{
// 		if (int status = CPXcallbackpostheursoln(context, cnt, indices.data(), incSol->sol.data(), incSol->oMIPCost, CPXCALLBACKSOLUTION_NOCHECK)) {
// 			PRINT_ERR("[OMIP] - Unable to set the heuristic - Code: %d", status);
// 			return 0;
// 		}
// 	}
	
// 	return 0;
// }


// int CPXPUBLIC updateIncMIPHandler(CPXCALLBACKCONTEXTptr context, CPXLONG contextid, void* inc) {
// 	switch (contextid) {
// 		case CPX_CALLBACKCONTEXT_CANDIDATE:
// 			return updatePoolsubMIP(context, inc);
// 		case CPX_CALLBACKCONTEXT_GLOBAL_PROGRESS:
// 			return updateLocalSolsubMIP(context, inc);
// 		default:
// 			PRINT_ERR("Contextid unknownn in updateFMIPHandler!");
// 			return 1;
// 	}
// }

// #pragma endregion

MTContext::MTContext(size_t subMIPNum, unsigned long long intialSeed) : numMIPs{ subMIPNum } {

	bestACSIncumbent = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
	// incumbentAmongMIPs = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND }; FIXME: v0.0.9 - remove it
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

	if (((sol.oMIPCost < bestACSIncumbent.oMIPCost) && (abs(bestACSIncumbent.slackSum) - abs(sol.slackSum)) <= EPSILON) || abs(sol.slackSum) < abs(bestACSIncumbent.slackSum)) {

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

MTContext& MTContext::parallelOMIPOptimization(double slackSumUB, Args& CLIArgs) {
	waitAllJobs();

	for (size_t i{ 0 }; i < CLIArgs.numsubMIPs; i++) {
		threads.emplace_back(&MTContext::OMIPInstanceJob, this, i, slackSumUB, std::ref(CLIArgs));
	}

	waitAllJobs();
	return *this;
}

// FIXME: v0.0.9 - remove it
// MTContext& MTContext::parallelInitSolMerge(std::string fileName, std::vector<double>& sol, Random& rnd) {

// 	waitAllJobs();

// 	for (size_t i{ 0 }; i < numMIPs; i++) {
// 		threads.emplace_back(&MTContext::initSolMergeJob, this, i, fileName);
// 	}

// 	waitAllJobs();

// 	std::vector<std::vector<double>> sols;
// 	sols.reserve(tmpSolutions.size());
// 	std::transform(tmpSolutions.begin(), tmpSolutions.end(), std::back_inserter(sols), [](const Solution& s) { return s.sol; });

// 	MIP	   MIP{ fileName };
// 	size_t numVarsToFix{ MIP.getMIPNumVars()};
// 	sol.resize(numVarsToFix, CPX_INFBOUND);

// 	for (size_t i{ 0 }; i < numVarsToFix; i++){
// 		double minLB = CPX_INFBOUND;
// 		double maxUB = -CPX_INFBOUND;
// 		for (size_t j{ 0 }; j < numMIPs; j++) {
// 			if(tmpSolutions[j].sol[i] > maxUB)
// 				maxUB = tmpSolutions[j].sol[i];
			
// 			if(tmpSolutions[j].sol[i] < minLB)
// 				minLB = tmpSolutions[j].sol[i];
// 		}

// 		MIP.setVarLowerBound(i, std::floor(minLB));
// 		MIP.setVarUpperBound(i, std::ceil(maxUB));
// 	}

// 	MIP.setNumCores(CPLEX_CORE).setNumSols(1);
// 	int error{ MIP.solve(CPX_INFBOUND, DET_TL(MIP.getNumNonZeros())) };
// 	if(MIP::isINForUNBD(error)){
// 		PRINT_WARN("MTContext: No Init Feasible sol found, generating a random one");
// 		FixPolicy::startSolTheta(sol, fileName, 1.0, rnd);
// 	} else {
// 		PRINT_INFO("MTContext: Feasible Init sol by multiple RENS");
// 		sol = MIP.getSol();
// 	}
// 	return *this;
// }

MTContext::~MTContext() {
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("MT Context: Closed");
#endif
}

void MTContext::FMIPInstanceJob(const size_t thID, Args& CLIArgs) {

	FMIP fMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		fMIP.addMIPStart(bestACSIncumbent.sol);
	}
	fMIP.setNumCores(CPLEX_CORE);
	// if (CLIArgs.algo == 2) FIXME: v0.0.9 - remove it
	// 	fMIP.setCallbackFunction(ACS_CB_CONTEXTMASK, updateIncMIPHandler, &incumbentAmongMIPs);

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

	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID, tmpSolutions[thID].slackSum);
	setBestACSIncumbent(tmpSolutions[thID]);

	//FixPolicy::dynamicAdjustRhoMT(thID, "FMIP", solveCode, numMIPs, CLIArgs.rho, A_RhoChanges);
}

#pragma region MTContextPrivateSec

void MTContext::OMIPInstanceJob(const size_t thID, const double slackSumUB, Args& CLIArgs) {

	OMIP oMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		oMIP.addMIPStart(bestACSIncumbent.sol);
	}
	oMIP.setNumCores(CPLEX_CORE);
	oMIP.updateBudgetConstr(slackSumUB);
	// if (CLIArgs.algo == 2) FIXME: v0.0.9 - remove it
	// 	oMIP.setCallbackFunction(ACS_CB_CONTEXTMASK, updateIncMIPHandler, &incumbentAmongMIPs);

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

	//FixPolicy::dynamicAdjustRhoMT(thID, "OMIP", solveCode, numMIPs, CLIArgs.rho, A_RhoChanges);
}

// FIXME: v0.0.9 - remove it
// void MTContext::initSolMergeJob(const size_t thID, std::string fileName) {

// 	RlxFMIP rlxFMIP{ fileName };
// 	rlxFMIP.setNumCores(CPLEX_CORE).setNumSols(1);

// 	rlxFMIP.solveRelaxation(CPX_INFBOUND, DET_TL(rlxFMIP.getNumNonZeros()));
// 	tmpSolutions[thID].sol = rlxFMIP.getSol();
// #if ACS_VERBOSE >= VERBOSE
// 	PRINT_INFO("Proc: %3d [Start_point] - MTContext::initSolMergeJob - New RLX sol found", thID);
// #endif
// }
#pragma endregion
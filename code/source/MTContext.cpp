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

	if(((sol.oMIPCost < bestACSIncumbent.oMIPCost) && (abs(bestACSIncumbent.slackSum ) - abs(sol.slackSum)) <= EPSILON) || abs(sol.slackSum) < abs(bestACSIncumbent.slackSum)){

		std::lock_guard<std::mutex> lock(MTContextMTX);
		bestACSIncumbent = { .sol = sol.sol, .slackSum = sol.slackSum, .oMIPCost = sol.oMIPCost };
		A_RhoChanges = numMIPs;

		if(bestACSIncumbent.oMIPCost< CPX_INFBOUND && bestACSIncumbent.slackSum<= EPSILON)
			PRINT_BEST("New MIP Incumbent found %12.2f\t[*]", bestACSIncumbent.oMIPCost);
			
	#if ACS_VERBOSE >= VERBOSE
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
	A_RhoChanges=0;
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


MTContext& MTContext::parallelInitSolMerge(std::string fileName, std::vector<double>& sol, Random& rnd){

	MIP	 MIP{ fileName };
	size_t	numVarsToFix{ static_cast<size_t> (MIP.getMIPNumVars()) };
	sol.resize(numVarsToFix, CPX_INFBOUND);

	std::vector<VarBounds> varRanges(numVarsToFix);
	for (size_t i{ 0 }; i < numVarsToFix; i++) varRanges[i] = MIP.getVarBounds(i);

	std::vector<double> obj = MIP.getObjFunction();

	waitAllJobs();

	for (size_t i{ 0 }; i < numMIPs; i++) {
		threads.emplace_back(&MTContext::initSolMergeJob, this, i, std::ref(varRanges), std::ref(obj));
	}

	waitAllJobs();

	std::vector<std::vector<double>> sols;
	sols.reserve(tmpSolutions.size()); 
	std::transform(tmpSolutions.begin(), tmpSolutions.end(), std::back_inserter(sols),[](const Solution& s) { return s.sol; });

	FixPolicy::fixMergeOnStartSol(numMIPs,numMIPs,sols, varRanges,rnd, sol);
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
	if (MIP::isINForUNBD(solveCode)){
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Proc: %3d [FMIP] - Aborted: Infeasible with given TL", thID);
#endif
		return;
	}

	tmpSolutions[thID].sol = fMIP.getSol();
	tmpSolutions[thID].slackSum = fMIP.getObjValue();

	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID, tmpSolutions[thID].slackSum);
	setBestACSIncumbent(tmpSolutions[thID]);

	FixPolicy::dynamicAdjustRhoMT(thID,"FMIP",solveCode,numMIPs,CLIArgs.rho,A_RhoChanges);
}


void MTContext::OMIPInstanceJob(const size_t thID, const double slackSumUB, Args& CLIArgs) {

	OMIP oMIP{ CLIArgs.fileName };
	if (bestACSIncumbent.slackSum < CPX_INFBOUND) {
		oMIP.addMIPStart(bestACSIncumbent.sol);
	}
	oMIP.setNumCores(CPLEX_CORE);
	oMIP.updateBudgetConstr(slackSumUB);

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

	FixPolicy::dynamicAdjustRhoMT(thID,"OMIP",solveCode,numMIPs,CLIArgs.rho,A_RhoChanges);
}


void MTContext::initSolMergeJob(const size_t thID, const std::vector<VarBounds>& vBounds, const std::vector<double>& obj){
	std::vector<std::vector<double>> draftSols(numMIPs);
	
	
	size_t lenSol = static_cast<size_t>(vBounds.size());
	tmpSolutions[thID].sol.resize(lenSol, CPX_INFBOUND);
	for (size_t i{ 0 }; i < numMIPs; i++)
		draftSols[i].resize(lenSol, CPX_INFBOUND);

	double min_obj = CPX_INFBOUND;
	for (size_t i{ 0 }; i < numMIPs; i++) {
		for(size_t j { 0 }; j < lenSol;j++){
			auto [lb, ub] = vBounds[j];
		
			double clampedLower = std::max(-MAX_UB, lb);
			double clampedUpper = std::min(MAX_UB, ub);
			draftSols[i][j] = rndGens[thID].Int(clampedLower, clampedUpper);
		}

		double objCost = std::inner_product(draftSols[i].begin(), draftSols[i].end(), obj.begin(), 0.0);
		if (objCost < min_obj) {
			std::lock_guard<std::mutex> lock(MTContextMTX);
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("Proc: %3d [Start_point] - MTContext::initSolMergeJob - New MIN sol \t %10.2f",thID,objCost); 
#endif
			min_obj = objCost;
			tmpSolutions[thID].sol = draftSols[i];
		}
	}
}
#include "../include/MTContext.hpp"

#define CPLEX_CORE 1

void FMIPInstanceJob(size_t thID, double remTime, Args CLIArgs, Solution& sol){
    
    FMIP fMIP{CLIArgs.fileName};
	fMIP.setNumCores(CPLEX_CORE);
   
	std::vector<size_t> varsToFix = randomRhoFix(sol.sol.size(), CLIArgs.rho, "FMIP", Random(10));

    for (auto i : varsToFix) {
		fMIP.setVarValue(i, sol.sol[i]);
	}

	int solveCode{fMIP.solve(remTime,CLIArgs.LNSDtimeLimit)};
    //TODO CHECK SolveCode

    sol.sol = fMIP.getSol();
    sol.slackSum = fMIP.getObjValue();
	PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", thID,sol.slackSum);
}



MTContext::MTContext(size_t subMIPNum, unsigned long long intialSeed) : 
    bestIncumbent{.sol = std::vector<double>() ,.oMIPCost=CPX_INFBOUND,.slackSum=CPX_INFBOUND} {

    tmpSolutions = std::vector<Solution>();
    threads = std::vector<std::thread>();

    threads.reserve(subMIPNum);
    tmpSolutions.reserve(subMIPNum);
    threadIDs.reserve(subMIPNum);
    rndGens.reserve(subMIPNum);

    for (size_t i {0}; i < subMIPNum; i++) {
        threadIDs.push_back(i); 
        rndGens.emplace_back(intialSeed+(i+1));
    }

#if ACS_VERBOSE >= VERBOSE
    PRINT_INFO("MT Context: Initialized -- Num schedulable jobs: %d",threadIDs.size());
#endif
}


MTContext& MTContext::setBestIncumbent(Solution sol){
    if(abs(sol.slackSum) > EPSILON){
        return *this;
    }

    if(sol.oMIPCost >= bestIncumbent.oMIPCost){
        return *this;
    }

    std::lock_guard<std::mutex> lock(updateSolMTX);
    bestIncumbent = {.sol = sol.sol, .slackSum = sol.slackSum, .oMIPCost = sol.oMIPCost};
#if ACS_VERBOSE >= VERBOSE
    PRINT_INFO("New incumbent found %12.2f|%-10.2f",bestIncumbent.oMIPCost, bestIncumbent.slackSum);
#endif
    return *this;
}

MTContext& MTContext::setTmpSolution(int index, Solution tmpSol){
    tmpSolutions[index]=tmpSol;
    return *this;
}

MTContext& MTContext::waitAllJobs() {
    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    threads.clear();
#if ACS_VERBOSE >= VERBOSE
    PRINT_INFO("MT Context: Join of threads");
#endif
    return *this;
}

MTContext& MTContext::parallelFMIPOptimization(double remTime, Args CLIArgs){
    waitAllJobs();
    
    for (size_t i = 0; i < CLIArgs.numsubMIPs; i++) {
        PRINT_WARN("Seed %u",rndGens[i].getSeed());
        //threads.emplace_back(FMIPInstanceJob,threadIDs[i],remTime,CLIArgs, std::ref(tmpSolutions[threadIDs[i]]);
    }
    waitAllJobs();
    return *this;
}


MTContext::~MTContext(){
#if ACS_VERBOSE >= VERBOSE
    PRINT_INFO("MT Context: Closed");
#endif
}


//void MTContext::OMIPInstanceJob(size_t thID, double remTime, const Args& CLIArgs)


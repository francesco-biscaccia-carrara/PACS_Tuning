/**
 * CPLEX Execution file 
 * 
 * @author Francesco Biscaccia Carrara
 * @version v1.1.3
 * @since 04/08/2025
*/

#include "../include/MIP.hpp"
#define CPLEX_RUN true
#define NUM_CORE 4

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args	  CLIArgs = CLIParser(argc, argv, CPLEX_RUN).getArgs();

        MIP ogMIP{CLIArgs.fileName};
		ogMIP.setNumCores(NUM_CORE).setNumSols(NUM_SOL_STOP);
        Solution CPLEXSol = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
        
        int solveCode {ogMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit))};

		if (MIP::isINForUNBD(solveCode)){
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
        }else{
            CPLEXSol.sol = ogMIP.getSol();
            CPLEXSol.oMIPCost = ogMIP.getObjValue();
            CPLEXSol.slackSum = 0.0;
            PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", CPLEXSol.oMIPCost, CPLEXSol.slackSum); 
        }
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
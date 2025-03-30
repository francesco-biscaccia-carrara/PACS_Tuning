/**
 * CPLEX Execution file 
 * 
 * @author Francesco Biscaccia Carrara
 * @version v1.0.3
 * @since 03/30/2025
*/

#include "../include/MIP.hpp"
#define CPLEX_RUN true

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args	  CLIArgs = CLIParser(argc, argv, CPLEX_RUN).getArgs();

        MIP ogMIP{CLIArgs.fileName};
        Solution CPLEXSol = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
        
        int solveCode {ogMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit))};

		if (solveCode == CPXMIP_TIME_LIM_INFEAS || solveCode == CPXMIP_DETTIME_LIM_INFEAS){
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
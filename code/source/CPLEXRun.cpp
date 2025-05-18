/**
 * CPLEX Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.9
 * @since 05/18/2025
 */

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../include/MIP.hpp"

#define PATH_TO_TMP "../test/scripts/tmp/"

#define CPLEX_RUN true
#define NUM_CORE 4

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args CLIArgs = CLIParser(argc, argv, CPLEX_RUN).getArgs();

		MIP ogMIP{ CLIArgs.fileName };
		ogMIP.setNumCores(NUM_CORE).setNumSols(NUM_SOL_STOP);
		Solution CPLEXSol = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };

		int solveCode{ ogMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit)) };
		double	 retTime = Clock::timeElapsed();
#if ACS_TEST
		nlohmann::json jsData;
#endif
		if (MIP::isINForUNBD(solveCode)) {
#if ACS_TEST
		jsData[CLIArgs.fileName]["CPLEX"]= { "NO SOL", retTime };
#endif
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			CPLEXSol.sol = ogMIP.getSol();
			CPLEXSol.oMIPCost = ogMIP.getObjValue();
			CPLEXSol.slackSum = 0.0;
#if ACS_TEST
		jsData[CLIArgs.fileName]["CPLEX"]= {CPLEXSol.oMIPCost, retTime };
#endif
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", CPLEXSol.oMIPCost, CPLEXSol.slackSum);
		}
#if ACS_TEST
		std::string	  JSfilename = CLIArgs.fileName + "_CPLEX.json";
		std::ofstream oFile(PATH_TO_TMP + JSfilename);

		oFile << jsData.dump(4);
		oFile.close();
		PRINT_INFO("JSON: Execution result saved on %s%s file",PATH_TO_TMP,JSfilename.c_str());
#endif
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
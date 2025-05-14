/**
 * CPLEX Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.8
 * @since 05/14/2025
 */

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../include/MIP.hpp"

#define ENV_FILE "../.ACSenv"
#define PATH_TO_JS "../test/scripts/"

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
#if ACS_TEST
		std::ifstream iFile(PATH_TO_JS+getJSONFilename(ENV_FILE));
		nlohmann::json j;
		iFile >> j;
		iFile.close();
#endif
		if (MIP::isINForUNBD(solveCode)) {
#if ACS_TEST
			j[CLIArgs.fileName]["CPLEX"] = "NO SOL";
#endif
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			CPLEXSol.sol = ogMIP.getSol();
			CPLEXSol.oMIPCost = ogMIP.getObjValue();
			CPLEXSol.slackSum = 0.0;
#if ACS_TEST
			j[CLIArgs.fileName]["CPLEX"] = CPLEXSol.oMIPCost;
#endif
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", CPLEXSol.oMIPCost, CPLEXSol.slackSum);
		}
#if ACS_TEST
		std::ofstream oFile(PATH_TO_JS+getJSONFilename(ENV_FILE));
		oFile << j.dump(4);
		oFile.close();
		PRINT_INFO("JSON: Execution result saved on %s file", getJSONFilename(ENV_FILE).c_str());
#endif
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
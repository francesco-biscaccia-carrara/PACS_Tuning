/**
 * CPLEX Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.7
 * @since 08/07/2025
 */

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../include/MIP.hpp"

#define JSON_FILE "../test/scripts/jsonTMP.json"
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
#if LOG >= 1
		std::ifstream iFile(JSON_FILE);
		nlohmann::json j;
		iFile >> j;
		iFile.close();
#endif
		if (MIP::isINForUNBD(solveCode)) {
#if LOG >= 1
			j[CLIArgs.fileName]["CPLEX"] = "NO SOL";
#endif
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			CPLEXSol.sol = ogMIP.getSol();
			CPLEXSol.oMIPCost = ogMIP.getObjValue();
			CPLEXSol.slackSum = 0.0;
#if LOG >= 1
			j[CLIArgs.fileName]["CPLEX"] = CPLEXSol.oMIPCost;
#endif
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", CPLEXSol.oMIPCost, CPLEXSol.slackSum);
		}
#if LOG >=1
		std::ofstream oFile(JSON_FILE);
		oFile << j.dump(4);
		oFile.close();
		PRINT_INFO("JSON: Execution result saved on %s file", JSON_FILE);
#endif
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}
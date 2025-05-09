/**
 * ACS Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.7
 * @since 05/09/2025
 */

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MTContext.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/OMIP.hpp"

#define ENV_FILE "../.env"
#define PATH_TO_JS "../test/scripts/"

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args	  CLIArgs = CLIParser(argc, argv).getArgs();
		MTContext MTEnv(CLIArgs.numsubMIPs, CLIArgs.seed);

		std::vector<double> startSol;
		Random				mainRnd = Random(CLIArgs.seed);

		switch (CLIArgs.algo) {
			case 0:
				PRINT_WARN("CLIArgs.theta = 1.0"); //ACS_Th-1_geomean
				FixPolicy::startSolTheta(startSol, CLIArgs.fileName, 1, mainRnd);
				break;

			case 1: //ACS_DetBounds_geomean
				FixPolicy::startSolMin(startSol, CLIArgs.fileName, mainRnd);
				break;

			case 2: //ACS_DetBoundsCB_geomean
				PRINT_WARN("Callbacks enabled!"); 
				FixPolicy::startSolMin(startSol, CLIArgs.fileName, mainRnd);
				break;

			case 3: //ACS_MultiRENS_geomean
				MTEnv.parallelInitSolMerge(CLIArgs.fileName, startSol, mainRnd);
			break;

			default:
				PRINT_ERR("Something goes wrong! Alg value: %d", CLIArgs.algo);
				break;
		}

		Solution tmpSol = { .sol = startSol, .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Starting vector found!");
#endif
		MTEnv.broadcastSol(tmpSol);
		MTEnv.setIncumbentAmongMIPsSize(FMIP(CLIArgs.fileName).getNumCols());

		while (Clock::timeElapsed() < CLIArgs.timeLimit) {
			if (abs(MTEnv.getBestACSIncumbent().slackSum) > EPSILON) {

				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}
				// PARALLEL FMIP Phase
				MTEnv.parallelFMIPOptimization(CLIArgs);
				if (MTEnv.isFeasibleSolFound())
					break;

				// 1° Recombination phase
				FMIP MergeFMIP(CLIArgs.fileName);
				MergeFMIP.setNumCores(CPLEX_CORE);

				MergePolicy::recombine(MergeFMIP, MTEnv.getTmpSolutions(), "1_Phase");

				if (MTEnv.getBestACSIncumbent().slackSum < CPX_INFBOUND) {
					MergeFMIP.addMIPStart(MTEnv.getBestACSIncumbent().sol);
				}

				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}

				int solveCode{ MergeFMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), DET_TL(MergeFMIP.getNumNonZeros())) };

				if (MIP::isINForUNBD(solveCode)) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("MergeOMIP - Aborted: Infeasible with given TL");
#endif
					continue;
				}
				tmpSol.sol = MergeFMIP.getSol();
				tmpSol.slackSum = MergeFMIP.getObjValue();
				PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);
				MTEnv.setBestACSIncumbent(tmpSol);
				FixPolicy::dynamicAdjustRho("1_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());

				if (MTEnv.isFeasibleSolFound())
					break;
				MTEnv.broadcastSol(tmpSol);
			}

			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}

			// PARALLEL OMIP Phase
			MTEnv.parallelOMIPOptimization(tmpSol.slackSum, CLIArgs);
			if (MTEnv.isFeasibleSolFound())
				break;

			// 2° Recombination phase
			OMIP MergeOMIP(CLIArgs.fileName);
			MergeOMIP.setNumCores(CPLEX_CORE);

			MergePolicy::recombine(MergeOMIP, MTEnv.getTmpSolutions(), "2_Phase");

			if (MTEnv.getBestACSIncumbent().slackSum < CPX_INFBOUND) {
				MergeOMIP.addMIPStart(MTEnv.getBestACSIncumbent().sol);
			}

			MergeOMIP.updateBudgetConstr(tmpSol.slackSum);
			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}
			int solveCode{ MergeOMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), DET_TL(MergeOMIP.getNumNonZeros())) };

			if (MIP::isINForUNBD(solveCode)) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("MergeOMIP - Aborted: Infeasible with given TL");
#endif
				continue;
			}

			tmpSol.sol = MergeOMIP.getSol();
			tmpSol.slackSum = MergeOMIP.getSlackSum();
			tmpSol.oMIPCost = MergeOMIP.getObjValue();

			PRINT_OUT("OptMIP Objective|SlackSum after merging: %12.2f|%-10.2f", MergeOMIP.getObjValue(), tmpSol.slackSum);
			MTEnv.setBestACSIncumbent(tmpSol);
			FixPolicy::dynamicAdjustRho("2_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());

			if (MTEnv.isFeasibleSolFound())
				break;
			MTEnv.broadcastSol(tmpSol);
		}

		Solution incumbent = MTEnv.getBestACSIncumbent();
#if ACS_TEST
		std::ifstream iFile(PATH_TO_JS+getJSONFilename(ENV_FILE));
		nlohmann::json j;
		iFile >> j;
		iFile.close();
#endif
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
#if ACS_TEST
			j[CLIArgs.fileName][std::to_string(CLIArgs.algo)][std::to_string(CLIArgs.seed)] = "NO SOL";
#endif
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			MIP og(CLIArgs.fileName);
			incumbent.sol.resize(og.getNumCols());
			//TODO: Check feas of solution
#if ACS_TEST
			j[CLIArgs.fileName][std::to_string(CLIArgs.algo)][std::to_string(CLIArgs.seed)]  = incumbent.oMIPCost;		
#endif
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", incumbent.oMIPCost, incumbent.slackSum);
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
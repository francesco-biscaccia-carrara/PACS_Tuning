/**
 * ACS Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.5
 * @since 06/16/2025
 */

#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MTContext.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/OMIP.hpp"

#define PATH_TO_TMP "../test/scripts/tmp/"

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args	  CLIArgs = CLIParser(argc, argv).getArgs();
		MTContext MTEnv(CLIArgs.numsubMIPs, CLIArgs.seed);

		std::vector<double> startSol;
		Random				mainRnd = Random(CLIArgs.seed);
		
		switch(CLIArgs.algo){
			case 0:
				CLIArgs.rho = 0.1;
			break;

			case 1:
				CLIArgs.rho = 0.25;
			break;

			case 2:
				CLIArgs.rho = 0.5;
			break;

			case 3:
				CLIArgs.rho = 0.75;
			break;

			case 4:
				CLIArgs.rho = 0.9;
			break;

			default:break;
		}
		PRINT_WARN("No Dyn -- Rho: %3.2f", CLIArgs.rho);

		FixPolicy::startSolTheta(startSol, CLIArgs.fileName, CLIArgs.theta, mainRnd);
		
		//FixPolicy::startSolMaxFeas(startSol, CLIArgs.fileName, mainRnd);
		Solution tmpSol = { .sol = startSol, .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Starting vector found!");
#endif
		MTEnv.broadcastSol(tmpSol);

		while (Clock::timeElapsed() < CLIArgs.timeLimit) {
			if (std::abs(MTEnv.getBestACSIncumbent().slackSum) > EPSILON) {

				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}
				// PARALLEL FMIP Phase
				MTEnv.parallelFMIPOptimization(CLIArgs);

				// 1° Recombination phase
				FMIP MergeFMIP(CLIArgs.fileName);
				MergeFMIP.setNumCores(CPLEX_CORE);

				MergePolicy::recombine(MergeFMIP, MTEnv.getTmpSolutions(), "1_Phase");

				if (MTEnv.getBestACSIncumbent().slackSum < CPX_INFBOUND) {	
					MergeFMIP.addMIPStart(MTEnv.getBestACSIncumbent().sol);
					//FixPolicy::fixSlackUpperBound("1_Phase", MergeFMIP, MTEnv.getBestACSIncumbent().sol);
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
					PRINT_INFO("MergeFMIP - Aborted: Infeasible with given TL");
#endif
					continue;
				}
				
				tmpSol.sol = MergeFMIP.getSol();
				tmpSol.slackSum = MergeFMIP.getObjValue();
				tmpSol.oMIPCost = MergeFMIP.getOMIPCost(tmpSol.sol);
				PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);
				MTEnv.setBestACSIncumbent(tmpSol);

				// FixPolicy::dynamicAdjustRho("1_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());
				MTEnv.broadcastSol(tmpSol);
			}

			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}

			// PARALLEL OMIP Phase
			MTEnv.parallelOMIPOptimization(CLIArgs,tmpSol.slackSum);
			if (MTEnv.isFeasibleSolFound())
				break;

			// 2° Recombination phase
			OMIP MergeOMIP(CLIArgs.fileName);
			MergeOMIP.setNumCores(CPLEX_CORE);

			MergePolicy::recombine(MergeOMIP, MTEnv.getTmpSolutions(), "2_Phase");
			MergeOMIP.updateBudgetConstr(tmpSol.slackSum);

			if (MTEnv.getBestACSIncumbent().slackSum < CPX_INFBOUND) {
				MergeOMIP.addMIPStart(MTEnv.getBestACSIncumbent().sol);
				// FixPolicy::fixSlackUpperBound("2_Phase", MergeOMIP, MTEnv.getBestACSIncumbent().sol);
			}

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
			// FixPolicy::dynamicAdjustRho("2_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());

			if (MTEnv.isFeasibleSolFound())
				break;
			MTEnv.broadcastSol(tmpSol);
		}

		
		Solution incumbent = MTEnv.getBestACSIncumbent();
#if ACS_TEST
		double	 retTime = Clock::timeElapsed();
		nlohmann::json jsData;
#endif
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
#if ACS_TEST
			jsData[CLIArgs.fileName][std::to_string(CLIArgs.algo)][std::to_string(CLIArgs.seed)] = { "NO SOL", retTime };
#endif
		} else {
			MIP og(CLIArgs.fileName);
			incumbent.sol.resize(og.getNumCols());

			double ABS_MaxViol = og.checkFeasibility(incumbent.sol);
			double ABS_MaxIntViol = og.checkIntegrality(incumbent.sol);
			double REL_ObjErr = REL_ERR(incumbent.oMIPCost, og.checkObjValue(incumbent.sol));

#if ACS_VERBOSE
			PRINT_INFO("-------------------------------TEST PHASE--------------------------------");
			PRINT_INFO("MIP::checkFeasibility\tMax Constraint  Violation:\t%11.10f", ABS_MaxViol);
			PRINT_INFO("MIP::checkIntegrality\tMax Integrality Violation:\t%11.10f", ABS_MaxIntViol);
			PRINT_INFO("MIP::checkObjValue\t\tRelative Objective Error :\t%11.10f", REL_ObjErr);
			PRINT_INFO("-------------------------------------------------------------------------");
#endif

			if(ABS_MaxViol > EPSILON){
				throw ACSException(ACSException::ExceptionType::General, "MIP::checkFeasibility:\tFAILED","ACSmain");
			}

			if(ABS_MaxIntViol > EPSILON){
				throw ACSException(ACSException::ExceptionType::General, "MIP::checkFeasibility:\tFAILED","ACSmain");
			}

			if(REL_ObjErr > EPSILON){
				throw ACSException(ACSException::ExceptionType::General, "MIP::chekObjValue:\tFAILED","ACSmain");
			}

			
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", incumbent.oMIPCost, incumbent.slackSum);
#if ACS_TEST
			jsData[CLIArgs.fileName][std::to_string(CLIArgs.algo)][std::to_string(CLIArgs.seed)] = { incumbent.oMIPCost, retTime };
#endif
		}
#if ACS_TEST
		std::string	  JSfilename = CLIArgs.fileName + "_ACS_" + std::to_string(CLIArgs.algo) + "_" + std::to_string(CLIArgs.seed) + ".json";
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
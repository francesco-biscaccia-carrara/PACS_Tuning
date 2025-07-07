/**
 * ACS Execution file
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.9
 * @since 07/06/2025
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

		// DEFAULT VAL SECTION
		CLIArgs.rho = 0.1;

		switch (CLIArgs.algo){

			case 1:
				CLIArgs.walkProb = 0.6;
			break;
			
			case 2:
				CLIArgs.walkProb = 0.7;
			break;

			case 3:
				CLIArgs.walkProb = 0.8;
			break;

			default:
			case 0:
			break;
		}

		PRINT_WARN("DYN, Rho 0.1, MaxFeas, UB, WalkP: %0.2f %s", CLIArgs.walkProb, (!CLIArgs.algo)? "[IGNORED]":"");
		FixPolicy::startSolMaxFeas(startSol, CLIArgs.fileName, mainRnd);

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("Starting vector found!");
#endif
		
		Solution tmpSol = { .sol = startSol, .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };
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
					FixPolicy::fixSlackUpperBound("1_Phase", MergeFMIP, MTEnv.getBestACSIncumbent().sol);
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

				FixPolicy::dynamicAdjustRho("1_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());
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

			// 2° Recombination phase
			OMIP MergeOMIP(CLIArgs.fileName);
			MergeOMIP.setNumCores(CPLEX_CORE);

			MergePolicy::recombine(MergeOMIP, MTEnv.getTmpSolutions(), "2_Phase");
			// MergeOMIP.updateBudgetConstr(tmpSol.slackSum);			v1.2.9 -- no need of this

			if (MTEnv.getBestACSIncumbent().slackSum < CPX_INFBOUND) {
				MergeOMIP.addMIPStart(MTEnv.getBestACSIncumbent().sol);
				FixPolicy::fixSlackUpperBound("2_Phase", MergeOMIP, MTEnv.getBestACSIncumbent().sol);
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
			FixPolicy::dynamicAdjustRho("2_Phase", solveCode, CLIArgs.numsubMIPs, CLIArgs.rho, MTEnv.getRhoChanges());

			if (MTEnv.isFeasibleSolFound())
				break;
			MTEnv.broadcastSol(tmpSol);
		}

		
		Solution incumbent = MTEnv.getBestACSIncumbent();
		double	 retTime = Clock::timeElapsed();
#if ACS_TEST
		nlohmann::json jsData;
#endif
		printf("--------------------------------------------------------------------------------\n");
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
			PRINT_ERR("No solution found within time-limit: %-10.4f",CLIArgs.timeLimit);
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
				throw ACSException(ACSException::ExceptionType::CheckFeasibilityFailed, "MIP::checkFeasibility:\tFAILED","ACSmain");
			}

			if(ABS_MaxIntViol > EPSILON){
				throw ACSException(ACSException::ExceptionType::CheckIntegralityFailed, "MIP::checkFeasibility:\tFAILED","ACSmain");
			}

			if(REL_ObjErr > EPSILON){
				throw ACSException(ACSException::ExceptionType::CheckObectiveFailed, "MIP::chekObjValue:\tFAILED","ACSmain");
			}

			PRINT_BEST("ACS Solution: %16.4f \n\t\t   Time elapsed: %-10.4f", incumbent.oMIPCost, retTime);
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
	} catch (const ACSException& ex) {
		PRINT_ERR(ex.what());
		return ex.getErrorCode();
	}
	printf("-------------------- ACS::Francesco Biscaccia Carrara ©2025 --------------------\n");
	return EXIT_SUCCESS;
}
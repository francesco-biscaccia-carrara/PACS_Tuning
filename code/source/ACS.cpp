/**
*   @author Francesco Biscaccia Carrara
*   
*   Last update: 03/29/2025
*/


#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MTContext.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/OMIP.hpp"
#include <assert.h>

int main(int argc, char* argv[]) {
	try {
		Clock::initTime = Clock::getTime();

		Args	  CLIArgs = CLIParser(argc, argv).getArgs();
		MTContext MTEnv(CLIArgs.numsubMIPs, CLIArgs.seed);

		std::vector<double> initSol;
		FixPolicy::firstThetaFixing(initSol,CLIArgs.fileName, CLIArgs.theta, Random(CLIArgs.seed));
		Solution tmpSol = {.sol = initSol, .slackSum = CPX_INFBOUND,  .oMIPCost = CPX_INFBOUND};
#if ACS_VERBOSE >= VERBOSE	
		PRINT_INFO("Init FeasMIP solution found!");
#endif
		MTEnv.broadcastSol(tmpSol);

		while (Clock::timeElapsed() < CLIArgs.timeLimit) {
			if (MTEnv.getBestACSIncumbent().slackSum > EPSILON) {

				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}
				MTEnv.parallelFMIPOptimization(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs);

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

				if (solveCode == CPXMIP_TIME_LIM_INFEAS || solveCode == CPXMIP_DETTIME_LIM_INFEAS || solveCode == CPXMIP_INFEASIBLE){
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("MergeOMIP - Aborted: Infeasible with given TL");
#endif
					continue;
				}
				tmpSol.sol = MergeFMIP.getSol();
				tmpSol.slackSum = MergeFMIP.getObjValue();
				PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);

				MTEnv.broadcastSol(tmpSol);
				MTEnv.setBestACSIncumbent(tmpSol);
			}

			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}

			MTEnv.parallelOMIPOptimization(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs, tmpSol.slackSum);

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
			int solveCode{ MergeOMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit),DET_TL(MergeOMIP.getNumNonZeros())) };

			if (solveCode == CPXMIP_TIME_LIM_INFEAS || solveCode == CPXMIP_DETTIME_LIM_INFEAS || solveCode == CPXMIP_INFEASIBLE){
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
			MTEnv.broadcastSol(tmpSol);
		}

		
		Solution incumbent = MTEnv.getBestACSIncumbent();
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			MIP		 og(CLIArgs.fileName);
			incumbent.sol.resize(og.getNumCols());
			assert(og.checkFeasibility(incumbent.sol) == true);
			PRINT_BEST("BEST INCUMBENT: %16.2f|%-10.2f", incumbent.oMIPCost, incumbent.slackSum);
		}

	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}
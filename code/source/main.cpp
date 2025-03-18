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

		Solution tmpSol = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };

		FMIP initFMIP(CLIArgs.fileName);
		tmpSol.sol.reserve(initFMIP.getMIPNumVars());
		std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
		FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta, Random(CLIArgs.seed));

		tmpSol.sol = initSol;
		tmpSol.slackSum = initFMIP.getObjValue();
		PRINT_OUT("Init FeasMIP solution cost: %25.2f", tmpSol.slackSum);

		MTEnv.broadcastSol(tmpSol);

		while (Clock::timeElapsed() < CLIArgs.timeLimit) {
			if (MTEnv.getBestIncumbent().slackSum > EPSILON) {

				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}
				MTEnv.parallelFMIPOptimization(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs);

				auto commonValues = MergePolicy::recombine(MTEnv.getTmpSolutions(), "1_Phase");

				FMIP MergeFMIP(CLIArgs.fileName);
				MergeFMIP.setNumCores(CPLEX_CORE);

				for (auto i : commonValues)
					MergeFMIP.setVarValue(i, MTEnv.getTmpSolution(0).sol[i]);
				if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
					PRINT_INFO("TIME_LIMIT REACHED");
#endif
					break;
				}
				MergeFMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs.LNSDtimeLimit);

				tmpSol.sol = MergeFMIP.getSol();
				tmpSol.slackSum = MergeFMIP.getObjValue();
				PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);

				MTEnv.broadcastSol(tmpSol);
			}
			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}
			MTEnv.parallelOMIPOptimization(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs, tmpSol.slackSum);

			auto commonValues = MergePolicy::recombine(MTEnv.getTmpSolutions(), "2_Phase");

			OMIP MergeOMIP(CLIArgs.fileName);
			MergeOMIP.setNumCores(CPLEX_CORE);

			for (auto i : commonValues)
				MergeOMIP.setVarValue(i, MTEnv.getTmpSolution(0).sol[i]);

			if (MTEnv.getBestIncumbent().slackSum < EPSILON) {
				std::vector<double> MIPStart(MTEnv.getBestIncumbent().sol);
				MIPStart.resize(MergeOMIP.getNumCols(), 0.0);
				MergeOMIP.addMIPStart(MIPStart);
			}

			MergeOMIP.updateBudgetConstr(tmpSol.slackSum);
			if (Clock::timeRemaining(CLIArgs.timeLimit) < EPSILON) {
#if ACS_VERBOSE >= VERBOSE
				PRINT_INFO("TIME_LIMIT REACHED");
#endif
				break;
			}
			int rtnValue{ MergeOMIP.solve(Clock::timeRemaining(CLIArgs.timeLimit), CLIArgs.LNSDtimeLimit) };

			if (rtnValue == CPXMIP_TIME_LIM_INFEAS)
				break;

			tmpSol.sol = MergeOMIP.getSol();
			tmpSol.slackSum = MergeOMIP.getSlackSum();
			tmpSol.oMIPCost = MergeOMIP.getObjValue();

			PRINT_OUT("OptMIP Objective|SlackSum after merging: %12.2f|%-10.2f", MergeOMIP.getObjValue(), tmpSol.slackSum);
			MTEnv.setBestIncumbent(tmpSol);
			MTEnv.broadcastSol(tmpSol);
		}

		MIP		 og(CLIArgs.fileName);
		Solution incumbent = MTEnv.getBestIncumbent();
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			assert(og.checkFeasibility(incumbent.sol) == true);
			PRINT_BEST("BEST INCUMBENT: %12.2f|%-10.2f", incumbent.oMIPCost, incumbent.slackSum);
		}
#if LOG
		Logger::closeFileLog();
#endif
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
	}

	return EXIT_SUCCESS;
}
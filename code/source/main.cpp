#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MTContext.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/OMIP.hpp"
#include <assert.h>

int main(int argc, char* argv[]) {
	try {
		Args	  CLIArgs = CLIParser(argc, argv).getArgs();
		MTContext MTEnv(CLIArgs.numsubMIPs, CLIArgs.seed);

		Solution tmpSol = { .sol = std::vector<double>(), .slackSum = CPX_INFBOUND, .oMIPCost = CPX_INFBOUND };

		FMIP initFMIP(CLIArgs.fileName);
		tmpSol.sol.reserve(initFMIP.getMIPNumVars());
		std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
		FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta, Random(CLIArgs.seed));

		tmpSol.sol = initSol;
		tmpSol.slackSum = initFMIP.getObjValue();
		PRINT_OUT("Init FeasMIP solution cost: %20.2f", tmpSol.slackSum);

		MTEnv.broadcastSol(tmpSol);

		double initTime = Clock::getTime();

		while (Clock::timeElapsed(initTime) < CLIArgs.timeLimit) {
			if (MTEnv.getBestIncumbent().slackSum > EPSILON) {

				MTEnv.parallelFMIPOptimization(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)), CLIArgs);

				auto commonValues = MergePolicy::recombine(MTEnv.getTmpSolutions(), "1_Phase");

				FMIP MergeFMIP(CLIArgs.fileName);
				MergeFMIP.setNumCores(CPLEX_CORE);

				for (auto i : commonValues)
					MergeFMIP.setVarValue(i, MTEnv.getTmpSolution(0).sol[i]);

				MergeFMIP.solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)), CLIArgs.LNSDtimeLimit);

				tmpSol.sol = MergeFMIP.getSol();
				tmpSol.slackSum = MergeFMIP.getObjValue();
				PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);

				MTEnv.broadcastSol(tmpSol);
			}

			MTEnv.parallelOMIPOptimization(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)), CLIArgs, tmpSol.slackSum);

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
			int rtnValue{ MergeOMIP.solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)), CLIArgs.LNSDtimeLimit) };

			if (rtnValue == CPXMIP_TIME_LIM_INFEAS)
				break;

			tmpSol.sol = MergeOMIP.getSol();
			tmpSol.slackSum = MergeOMIP.getSlackSum();
			tmpSol.oMIPCost = MergeOMIP.getObjValue();

			PRINT_OUT("OptMIP Objective|SlackSum after merging: %12.2f|%-10.2f", MergeOMIP.getObjValue(), tmpSol.slackSum);
			MTEnv.setBestIncumbent(tmpSol);
		}

		MIP		 og(CLIArgs.fileName);
		Solution incumbent = MTEnv.getBestIncumbent();
		if (incumbent.sol.empty() || incumbent.slackSum > EPSILON) {
			PRINT_ERR("NO FEASIBLE SOLUTION FIND");
		} else {
			assert(og.checkFeasibility(incumbent.sol) == true);
			PRINT_BEST("BEST INCUMBENT: %12.2f|%-10.2f", incumbent.oMIPCost, incumbent.slackSum);
		}
	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
	}
	return EXIT_SUCCESS;
}
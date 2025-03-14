#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MPIContext.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/OMIP.hpp"
#include <assert.h>

int main(int argc, char* argv[]) {
	MPIContext MPIEnv(argc, argv);

	try {
		Args				CLIArgs;
		Solution			tmpSol;
		std::vector<double> gatheredSol;

		if (MPIEnv.isMasterProcess()) {
			CLIParser CLIEnv(argc, argv);
			CLIArgs = CLIEnv.getArgs();
		}

		MPIEnv.barrier().broadcast(CLIArgs).barrier();

		Random::setSeed(CLIArgs.seed + MPIEnv.getRank());

		//<-- Initialize [x,Slack] as IntegerSol
		if (MPIEnv.isMasterProcess()) {
			FMIP initFMIP(CLIArgs.fileName);
			tmpSol.sol.reserve(initFMIP.getMIPNumVars());
			std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
			FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta);

			tmpSol.sol = initSol;
			tmpSol.slackSum = initFMIP.getObjValue();
			PRINT_OUT("Init FeasMIP solution cost: %20.2f", tmpSol.slackSum);
		}

		MPIEnv.barrier().broadcast(tmpSol).barrier();
		//<-- end

		double initTime = Clock::getTime();

		while (Clock::timeElapsed(initTime) < CLIArgs.timeLimit) {
			if (tmpSol.slackSum > EPSILON) {
				//<-- FMIP in parallel
				FMIP fMIP{CLIArgs.fileName};
				fMIP.setNumCores(CLIArgs.numsubMIPs);

				std::vector<size_t> varsToFix = FixPolicy::randomRhoFix(tmpSol.sol, CLIArgs.rho, MPIEnv.getRank(), "FMIP");
				for (auto i : varsToFix) {
					fMIP.setVarValue(i, tmpSol.sol[i]);
				}

				fMIP.solve(CLIArgs.LNSDtimeLimit);

				tmpSol.sol = fMIP.getSol();
				tmpSol.slackSum = fMIP.getObjValue();
				PRINT_OUT("Proc: %3d - FeasMIP Objective: %20.2f", MPIEnv.getRank(), tmpSol.slackSum);

				MPIEnv.barrier();
				//<-- end

				if (MPIEnv.isMasterProcess()) {
					std::vector<double> gatherSol_root;
					gatherSol_root.reserve((MPIEnv.getRank() * tmpSol.sol.size()));
					gatheredSol = gatherSol_root;
				}

				MPIEnv.barrier().gather(tmpSol.sol, gatheredSol).barrier();

				//<-- Recombination phase
				if (MPIEnv.isMasterProcess()) {
					auto commonValues = MergePolicy::recombine(gatheredSol, MPIEnv.getSize(), "1_Phase");
					FMIP MergeFMIP(CLIArgs.fileName);

					for (auto [i, value] : commonValues)
						MergeFMIP.setVarValue(i, value);

					MergeFMIP.solve(CLIArgs.LNSDtimeLimit);

					tmpSol.sol = MergeFMIP.getSol();
					tmpSol.slackSum = MergeFMIP.getObjValue();
					PRINT_OUT("FeasMIP Objective after merging: %20.2f", tmpSol.slackSum);
				}
				//<-- end
				MPIEnv.barrier().broadcast(tmpSol).barrier();
			}

			double slackSumUB{ tmpSol.slackSum };
			//<-- OMIP in parallel
			OMIP oMIP{CLIArgs.fileName};
			oMIP.setNumCores(CLIArgs.numsubMIPs);


			std::vector<size_t> varsToFix = FixPolicy::randomRhoFix(tmpSol.sol, CLIArgs.rho, MPIEnv.getRank(), "OMIP");
			for (auto i : varsToFix) {
				oMIP.setVarValue(i, tmpSol.sol[i]);
			}

			oMIP.updateBudgetConstr(slackSumUB);
			oMIP.solve(CLIArgs.LNSDtimeLimit);

			tmpSol.sol = oMIP.getSol();

			PRINT_OUT("Proc: %3d - OptMIP Objective: %20.2f", MPIEnv.getRank(), oMIP.getObjValue());
			MPIEnv.barrier();
			//<-- end

			MPIEnv.gather(tmpSol.sol, gatheredSol).barrier();

			//<-- Recombination phase
			if (MPIEnv.isMasterProcess()) {
				auto commonValues = MergePolicy::recombine(gatheredSol, MPIEnv.getSize(), "2_Phase");
				OMIP MergeOMIP(CLIArgs.fileName);

				for (auto [i, value] : commonValues)
					MergeOMIP.setVarValue(i, value);

				MergeOMIP.updateBudgetConstr(slackSumUB);
				MergeOMIP.solve(CLIArgs.LNSDtimeLimit);

				tmpSol.sol = MergeOMIP.getSol();
				tmpSol.slackSum = MergeOMIP.getSlackSum();

				PRINT_OUT("OptMIP Objective/SlackSum after merging: %12.2f|%-10.2f", MergeOMIP.getObjValue(), tmpSol.slackSum);
			}

			MPIEnv.barrier().broadcast(tmpSol).barrier();
		}

		if (MPIEnv.isMasterProcess()) {
			MIP og(CLIArgs.fileName);
			assert(og.checkFeasibility(tmpSol.sol) == true);
			PRINT_WARN("[FEASIBILITY_TEST]: PASSED");
		}

	} catch (const std::runtime_error& ex) {
		PRINT_ERR(ex.what());
		MPIEnv.abort();
	}

	return EXIT_SUCCESS;
}
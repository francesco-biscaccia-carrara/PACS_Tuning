#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/MPIContext.hpp"
#include "../include/OMIP.hpp"

int main(int argc, char* argv[]) {
	MPIContext MPIEnv(argc, argv);

	try {
		Args				CLIArgs;
		Solution 			tmpSol;
		std::vector<double> gatheredSol;

		if (MPIEnv.isMasterProcess()) {
			CLIParser CLIEnv(argc, argv);
			CLIArgs = CLIEnv.getArgs();
		}

		MPIEnv.barrier().broadcast(CLIArgs).barrier();

		Random::setSeed(CLIArgs.seed + MPIEnv.getRank());

		//<-- Initialize [x,Slack] as IntegerSol
		if (MPIEnv.isMasterProcess()) {
			FMIP				initFMIP(CLIArgs.fileName);
			tmpSol.sol.reserve(initFMIP.getMIPNumVars());
			std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
			FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta, CPX_INFBOUND);
			tmpSol.sol = initSol;
			tmpSol.slackSum = initFMIP.getObjValue();
			Logger::print(Logger::LogLevel::OUT, "Init FeasMIP solution cost: %20.2f", tmpSol.slackSum);
		}

		MPIEnv.barrier().broadcast(tmpSol).barrier();
		//<-- end
		
		double initTime = Clock::getTime();
	
		while (Clock::timeElapsed(initTime) < CLIArgs.timeLimit){
			if(tmpSol.slackSum > EPSILON){
				//<-- FMIP in parallel
				FMIP   fMIP(CLIArgs.fileName);
				
				FixPolicy::randomRhoFix(tmpSol.sol, CLIArgs.rho, MPIEnv.getRank(),"FMIP");
				tmpSol.sol.resize(fMIP.getNumCols(), CPX_INFBOUND);

				fMIP.setVarsValues(tmpSol.sol).solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)) / 2);
				
				tmpSol.sol= fMIP.getSol();
				tmpSol.slackSum = fMIP.getObjValue();
				Logger::print(Logger::LogLevel::OUT, "Proc: %3d - FeasMIP Objective: %20.2f %s", MPIEnv.getRank(), tmpSol.slackSum, (tmpSol.slackSum > -EPSILON && tmpSol.slackSum < EPSILON)? "<-":"");

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
					auto commonValues = MergePolicy::recombine(gatheredSol,MPIEnv.getSize(),"1_Phase");	
					FMIP MergeFMIP(CLIArgs.fileName);
			
					std::vector<double> varToFix(MergeFMIP.getNumCols(),CPX_INFBOUND);
					for(auto [i,value]: commonValues)
						varToFix[i]=value;
					
					//if(commonValues.size()<=MergeFMIP.getMIPNumVars()*0.5)
					MergeFMIP.setVarsValues(varToFix);
			
					MergeFMIP.solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)));
	
					tmpSol.sol = MergeFMIP.getSol();
					tmpSol.slackSum = MergeFMIP.getObjValue();
					Logger::print(Logger::LogLevel::OUT, "FeasMIP Objective after merging: %20.2f %s", tmpSol.slackSum, (tmpSol.slackSum > -EPSILON && tmpSol.slackSum < EPSILON)? "<-":"");
				}
				//<-- end
				MPIEnv.barrier().broadcast(tmpSol).barrier();
			}
			
			double slackSumUB{tmpSol.slackSum};
			//<-- OMIP in parallel
			OMIP   oMIP(CLIArgs.fileName);
			FixPolicy::randomRhoFix(tmpSol.sol, CLIArgs.rho, MPIEnv.getRank(),"OMIP");
			tmpSol.sol.resize(oMIP.getNumCols(), CPX_INFBOUND);

			oMIP.updateBudgetConstr(slackSumUB).setVarsValues(tmpSol.sol);
			oMIP.solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime))/ 2);

			tmpSol.sol = oMIP.getSol();

			Logger::print(Logger::LogLevel::OUT, "Proc: %3d - OptMIP Objective: %20.2f", MPIEnv.getRank(), oMIP.getObjValue());
			MPIEnv.barrier();
			//<-- end 

			MPIEnv.gather(tmpSol.sol, gatheredSol).barrier();

			//<-- Recombination phase
			if (MPIEnv.isMasterProcess()){
				auto commonValues = MergePolicy::recombine(gatheredSol,MPIEnv.getSize(),"2_Phase");
				OMIP MergeOMIP(CLIArgs.fileName);	

				std::vector<double> varToFix(MergeOMIP.getNumCols(),CPX_INFBOUND);
				for(auto [i,value]: commonValues)
					varToFix[i]=value;

				MergeOMIP.updateBudgetConstr(slackSumUB).setVarsValues(varToFix);
				MergeOMIP.solve(abs(CLIArgs.timeLimit - Clock::timeElapsed(initTime)));

				tmpSol.sol  = MergeOMIP.getSol();
				tmpSol.slackSum =MergeOMIP.getSlackSum();
				Logger::print(Logger::LogLevel::OUT, "OptMIP Objective after merging: %20.2f", MergeOMIP.getObjValue());
				Logger::print(Logger::LogLevel::OUT, "OptMIP SlackSum after merging: %20.2f", tmpSol.slackSum );
				
			}

			MPIEnv.barrier().broadcast(tmpSol).barrier();
		}

		if(MPIEnv.isMasterProcess()){
			MIP og(CLIArgs.fileName);
			assert(og.checkFeasibility(tmpSol.sol)==true);
			Logger::print(Logger::LogLevel::WARN,"[FEASIBILITY_TEST]: PASSED");
		}
	
	} catch (const std::runtime_error& ex) {
		Logger::print(Logger::LogLevel::ERROR, ex.what());
		MPIEnv.abort();
	}

	/*
	switch (STATE) {
		case CPXMIP_TIME_LIM_FEAS:      // exceeded time limit, found intermediate
	solution Logger::print(WARN,"exceeded time limit, intermediate solution
	found."); break; case CPXMIP_TIME_LIM_INFEAS:    // exceeded time limit, no
	intermediate solution found Logger::print(WARN,"exceeded time limit, no
	intermediate solution found."); break; case CPXMIP_INFEASIBLE:         //
	proven to be infeasible Logger::print(Logger::LogLevel::ERROR,"infeasible problem."); break;
		case CPXMIP_ABORT_FEAS:         // terminated by user, found solution
			Logger::print(WARN,"terminated by user, found solution found.");
			break;
		case CPXMIP_ABORT_INFEAS:       // terminated by user, not found solution
			Logger::print(WARN,"terminated by user, no solution found.");
			break;
		case CPXMIP_OPTIMAL_TOL:        // found optimal within the tollerance
			Logger::print(WARN,"found optimal within the tollerance.");
			break;
		case CPXMIP_OPTIMAL:            // found optimal
			Logger::print(WARN,"found optimal.");
			break;
		default:                        // unhandled status
			Logger::print(Logger::LogLevel::ERROR,"Unhandled cplex status: %d", STATE);
			break;
	}
	*/
	return EXIT_SUCCESS;
}
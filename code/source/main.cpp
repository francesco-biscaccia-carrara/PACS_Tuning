#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MergePolicy.hpp"
#include "../include/MPIContext.hpp"
#include "../include/OMIP.hpp"

int main(int argc, char* argv[]) {
	MPIContext MPIEnv(argc, argv);

	try {
		Args				CLIArgs;
		double FMIPCost {CPX_INFBOUND};
		double OMIPCost {CPX_INFBOUND};
		std::vector<double> tmpIntegerSol;
		std::vector<double> gatheredSol;
		int CPXTerminate {0};

		if (MPIEnv.isMasterProcess()) {
			CLIParser CLIEnv(argc, argv);
			CLIArgs = CLIEnv.getArgs();
		}

		MPIEnv.barrier();
		MPIEnv.broadcast(CLIArgs);
		MPIEnv.barrier();

		Random::setSeed(CLIArgs.seed + MPIEnv.getRank());

		//<-- Initialize [x,Slack] as IntegerSol
		if (MPIEnv.isMasterProcess()) {
			FMIP				initFMIP(CLIArgs.fileName);
			tmpIntegerSol.reserve(initFMIP.getMIPNumVars());
			std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
			FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta, CPX_INFBOUND);
			tmpIntegerSol = initSol;
			FMIPCost = initFMIP.getObjValue();
			Logger::print(Logger::LogLevel::OUT, "Init FeasMIP solution cost: %20.2f", FMIPCost);
		}
		MPIEnv.barrier();
		MPIEnv.broadcast(FMIPCost);
		MPIEnv.broadcast(tmpIntegerSol);
		MPIEnv.barrier();
		//<-- end
		

		double initTime = Clock::getTime();
	
		while (Clock::timeElapsed(initTime) < CLIArgs.timeLimit){

			if(FMIPCost > EPSILON){
				double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
				FMIP   fMIP(CLIArgs.fileName);
				fMIP.setTerminate(&CPXTerminate);
				
				FixPolicy::randomRhoFix(tmpIntegerSol, CLIArgs.rho, MPIEnv.getRank());
				tmpIntegerSol.resize(fMIP.getNumCols(), CPX_INFBOUND);
				fMIP.setVarsValues(tmpIntegerSol).solve(remainingTime / 2);

				if(Clock::timeElapsed(initTime) >= CLIArgs.timeLimit) CPXTerminate=1;		

				FMIPCost = fMIP.getObjValue();
				Logger::print(Logger::LogLevel::OUT, "Proc: %3d - FeasMIP Objective: %20.2f %s", MPIEnv.getRank(), FMIPCost, (FMIPCost > -EPSILON && FMIPCost < EPSILON)? "<-":"");

				MPIEnv.barrier();
				tmpIntegerSol = fMIP.getSol();
				MPIEnv.barrier();

				
				if (MPIEnv.isMasterProcess()) {
					std::vector<double> gatherSol_root;
					gatherSol_root.reserve((MPIEnv.getRank() * tmpIntegerSol.size()));
					gatheredSol = gatherSol_root;
				}
				
				MPIEnv.barrier();
				MPIEnv.gather(tmpIntegerSol, gatheredSol).barrier();

				if (MPIEnv.isMasterProcess()) {
					auto commonValues = MergePolicy::recombine(tmpIntegerSol,MPIEnv.getSize());	
					FMIP MergeFMIP(CLIArgs.fileName);
					MergeFMIP.setTerminate(&CPXTerminate);
			
					std::vector<double> varToFix(MergeFMIP.getNumCols(),CPX_INFBOUND);
					for(auto [i,value]: commonValues)
						varToFix[i]=value;
					if(commonValues.size()<=MergeFMIP.getMIPNumVars()*0.5)
						MergeFMIP.setVarsValues(varToFix);
					double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
					MergeFMIP.solve(CLIArgs.timeLimit - Clock::timeElapsed(initTime));
					if(Clock::timeElapsed(initTime) >= CLIArgs.timeLimit) CPXTerminate=1;	
					tmpIntegerSol = MergeFMIP.getSol();
					FMIPCost=MergeFMIP.getObjValue();
					Logger::print(Logger::LogLevel::OUT, "FeasMIP Objective after merging: %20.2f %s", FMIPCost, (FMIPCost > -EPSILON && FMIPCost < EPSILON)? "<-":"");
				}

				MPIEnv.broadcast(FMIPCost).barrier();
		}

			double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
			OMIP   oMIP(CLIArgs.fileName);
			oMIP.setTerminate(&CPXTerminate);

			FixPolicy::randomRhoFix(tmpIntegerSol, CLIArgs.rho, MPIEnv.getRank());
			tmpIntegerSol.resize(oMIP.getNumCols(), CPX_INFBOUND);
			
			oMIP.updateBudgetConstr(FMIPCost).setVarsValues(tmpIntegerSol);
			oMIP.solve(remainingTime / 2);
			if(Clock::timeElapsed(initTime) >= CLIArgs.timeLimit) CPXTerminate=1;	

			OMIPCost = oMIP.getObjValue();
			Logger::print(Logger::LogLevel::OUT, "Proc: %3d - OptMIP Objective: %20.2f", MPIEnv.getRank(), OMIPCost);

			tmpIntegerSol = oMIP.getSol();

			if (MPIEnv.isMasterProcess()) {
				auto commonValues = MergePolicy::recombine(tmpIntegerSol,MPIEnv.getSize());	
				OMIP MergeOMIP(CLIArgs.fileName);
				MergeOMIP.setTerminate(&CPXTerminate);

				std::vector<double> varToFix(MergeOMIP.getNumCols(),CPX_INFBOUND);
				for(auto [i,value]: commonValues)
					varToFix[i]=value;
				if(commonValues.size()<=MergeOMIP.getMIPNumVars()*0.5)
					MergeOMIP.setVarsValues(varToFix);
				MergeOMIP.solve(CLIArgs.timeLimit - Clock::timeElapsed(initTime));
				if(Clock::timeElapsed(initTime) >= CLIArgs.timeLimit) CPXTerminate=1;
				tmpIntegerSol = MergeOMIP.getSol();
				OMIPCost=MergeOMIP.getObjValue();
				Logger::print(Logger::LogLevel::OUT, "OptMIP Objective after merging: %20.2f", OMIPCost);
			}
		}

// 	{if(FMIPCost < -EPSILON || FMIPCost > EPSILON){
		
// 		while (
// 			if(FMIPCost < -EPSILON || FMIPCost > EPSILON){

// 		} &&
// 			   Clock::timeElapsed(initTime) < CLIArgs.timeLimit) {
			
// 			double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
// 			FMIP   fMIP(CLIArgs.fileName);
// 			fMIP.setTerminate(&CPXTerminate);
// 			FixPolicy::randomRhoFix(tmpIntegerSol, CLIArgs.rho, MPIEnv.getRank());
// 			tmpIntegerSol.resize(fMIP.getNumCols(), CPX_INFBOUND);
// 			fMIP.setVarsValues(tmpIntegerSol).solve(remainingTime / 2);

// 			if(Clock::timeElapsed(initTime) >= CLIArgs.timeLimit) CPXTerminate=1;		

// 			FMIPCost = fMIP.getObjValue();
// 			Logger::print(Logger::LogLevel::OUT, "Proc: %3d - FeasMIP Objective: %20.2f %s", MPIEnv.getRank(), FMIPCost, (FMIPCost > -EPSILON && FMIPCost < EPSILON)? "<-":"");

// 			tmpIntegerSol = fMIP.getSol();
// 		}
// 		MPIEnv.barrier();
	
// #if ACS_VERBOSE >= VERBOSE
// 		//MIP ogProb(CLIArgs.fileName);
// 		//assert(ogProb.checkFeasibility(tmpIntegerSol)==true);
// #endif

// 		std::vector<double> gatherSol;
// 		if (MPIEnv.isMasterProcess()) {
// 			std::vector<double> gatherSol_root;
// 			gatherSol_root.reserve((MPIEnv.getRank() * tmpIntegerSol.size()));
// 			gatherSol = gatherSol_root;
// 		}
		
// 		MPIEnv.barrier();
// 		MPIEnv.gather(tmpIntegerSol, gatherSol).barrier();

// 		double FMIPCost{CPX_INFBOUND};
// 		if (MPIEnv.isMasterProcess()) {
// 			auto commonValues = MergePolicy::recombine(tmpIntegerSol,MPIEnv.getSize());	
// 			FMIP MergeFMIP(CLIArgs.fileName);
// 			std::vector<double> varToFix(MergeFMIP.getNumCols(),CPX_INFBOUND);
// 			for(auto [i,value]: commonValues)
// 				varToFix[i]=value;
// 			if(commonValues.size()<=MergeFMIP.getMIPNumVars()*0.5)
// 				MergeFMIP.setVarsValues(varToFix);
// 			MergeFMIP.solve(CLIArgs.timeLimit - Clock::timeElapsed(initTime));
// 			tmpIntegerSol = MergeFMIP.getSol();
// 			FMIPCost=MergeFMIP.getObjValue();
// 			Logger::print(Logger::LogLevel::OUT, "FeasMIP Objective after merging: %20.2f %s", FMIPCost, (FMIPCost > -EPSILON && FMIPCost < EPSILON)? "<-":"");
// 		}

// 		#if ACS_VERBOSE >= VERBOSE
// 		//MIP nogProb(CLIArgs.fileName);
// 		//assert(nogProb.checkFeasibility(tmpIntegerSol)==true);
// #endif
// MPIEnv.broadcast(FMIPCost).barrier();
// MPIEnv.broadcast(tmpIntegerSol).barrier();

// double OMIPCost = CPX_INFBOUND;
// while ((OMIPCost < -EPSILON || OMIPCost > EPSILON) &&
// 			   Clock::timeElapsed(initTime) < CLIArgs.timeLimit) {
// 			double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
// 			OMIP   oMIP(CLIArgs.fileName);

// 			FixPolicy::randomRhoFix(tmpIntegerSol, CLIArgs.rho, MPIEnv.getRank());
// 			tmpIntegerSol.resize(oMIP.getNumCols(), CPX_INFBOUND);
			
// 			oMIP.updateBudgetConstr(OMIPCost).setVarsValues(tmpIntegerSol);
// 			oMIP.solve(remainingTime / 2);

// 			OMIPCost = oMIP.getObjValue();
// 			Logger::print(Logger::LogLevel::OUT, "Proc: %3d - OptMIP Objective: %20.2f", MPIEnv.getRank(), OMIPCost);

// 			tmpIntegerSol = oMIP.getSol();
// 		}
// 		MPIEnv.barrier();
// 		MPIEnv.gather(tmpIntegerSol, gatherSol).barrier();

// 		if (MPIEnv.isMasterProcess()) {
// 			auto commonValues = MergePolicy::recombine(tmpIntegerSol,MPIEnv.getSize());	
// 			OMIP MergeOMIP(CLIArgs.fileName);
// 			std::vector<double> varToFix(MergeOMIP.getNumCols(),CPX_INFBOUND);
// 			for(auto [i,value]: commonValues)
// 				varToFix[i]=value;
// 			if(commonValues.size()<=MergeOMIP.getMIPNumVars()*0.5)
// 				MergeOMIP.setVarsValues(varToFix);
// 			MergeOMIP.solve(CLIArgs.timeLimit - Clock::timeElapsed(initTime));
// 			tmpIntegerSol = MergeOMIP.getSol();
// 			FMIPCost=MergeOMIP.getObjValue();
// 			Logger::print(Logger::LogLevel::OUT, "OptMIP Objective after merging: %20.2f", FMIPCost);
// 		}}

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
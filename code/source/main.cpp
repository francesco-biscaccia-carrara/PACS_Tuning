#include "../include/FMIP.hpp"
#include "../include/FixPolicy.hpp"
#include "../include/MPIContext.hpp"
#include "../include/OMIP.hpp"

int main(int argc, char* argv[]) {
	MPIContext MPIEnv(argc, argv);

	try {
		Args				CLIArgs;
		std::vector<double> initIntegerSol;

		if (MPIEnv.isMasterProcess()) {
			CLIParser CLIEnv(argc, argv);
			CLIArgs = CLIEnv.getArgs();
			Random::setSeed(CLIArgs.seed);
			FMIP				initFMIP(CLIArgs.fileName);
			std::vector<double> initSol(initFMIP.getMIPNumVars(), CPX_INFBOUND);
			FixPolicy::firstThetaFixing(initFMIP, initSol, CLIArgs.theta, CLIArgs.timeLimit);
			initIntegerSol = initSol;
		}
		MPIEnv.barrier();

		MPIEnv.broadcast(CLIArgs).barrier();
		MPIEnv.broadcast(initIntegerSol).barrier();

		if (!MPIEnv.isMasterProcess())
			Random::setSeed(CLIArgs.seed + MPIEnv.getRank());

		double FMIPCost = CPX_INFBOUND;
		double initTime = Clock::getTime();

		while ((FMIPCost < -EPSILON || FMIPCost > EPSILON) &&
			   Clock::timeElapsed(initTime) < CLIArgs.timeLimit) {
			double remainingTime{ CLIArgs.timeLimit - Clock::timeElapsed(initTime) };
			FMIP   fMIP(CLIArgs.fileName);
			fMIP.setNumCores(4);
			FixPolicy::randomRhoFix(initIntegerSol, CLIArgs.rho, MPIEnv.getRank());
			initIntegerSol.resize(fMIP.getNumCols(), CPX_INFBOUND);
			fMIP.setVarsValues(initIntegerSol).solve(remainingTime / 2);
			FMIPCost = fMIP.getObjValue();
			Logger::print(Logger::LogLevel::OUT, "Proc: %3d - FeasMIP Objective: %20.2f", MPIEnv.getRank(), FMIPCost);
		}

		// 			fMIP.solve(CLIEnv.getTimeLimit());

		// #if ACS_VERBOSE == DEBUG
		// 		originalMIP.saveModel();
		// 		fMIP.saveModel().saveLog();
		// 		oMIP.saveModel().saveLog();
		// #endif

		// 		int					xLength = originalMIP.getNumCols();
		// 		std::vector<double> initSol(xLength, CPX_INFBOUND);
		// 		FixPolicy::fixTest(initSol);

		// 		double FMIPCost = CPX_INFBOUND;
		// 		double initTime = Clock::getTime();

		// 		while ((FMIPCost < -EPSILON || FMIPCost > EPSILON) &&
		// 			   Clock::timeElapsed(initTime) < CLIEnv.getTimeLimit()) {
		// 			FixPolicy::randomRhoFix(initSol, CLIEnv.getRho());
		// 			initSol.resize(fMIP.getNumCols(), CPX_INFBOUND);
		// 			fMIP.setVarsValues(initSol);
		// 			fMIP.solve(CLIEnv.getTimeLimit());
		// 			FMIPCost = fMIP.getObjValue();
		// 			std::cout << "FMIP cost:" << FMIPCost << std::endl;
		// 			std::vector<double> first_sol = fMIP.getSol();
		// 			FixPolicy::randomRhoFix(first_sol, CLIEnv.getRho());
		// 			first_sol.resize(fMIP.getNumCols(), CPX_INFBOUND);
		// 			oMIP.setVarsValues(first_sol);
		// 			oMIP.updateBudgetConstr(fMIP.getObjValue());
		// 			oMIP.solve(CLIEnv.getTimeLimit());
		// 			std::cout << "OMIP cost:" << oMIP.getObjValue() << std::endl;
		// 			initSol = oMIP.getSol();
		// 		}
	} catch (const std::runtime_error& ex) {
		if (MPIEnv.isMasterProcess())
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
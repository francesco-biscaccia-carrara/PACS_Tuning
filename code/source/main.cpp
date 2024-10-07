#include "../include/FMIP.hpp"
#include "../include/OMIP.hpp"
#include "../include/FixPolicy.hpp"

#define SEED 2120934

int main(int argc, char* argv[]){

    ArgsParser CLIEnv(argc,argv);
    MIP originalMIP(CLIEnv.getFileName());
    RandNumGen::setSeed(CLIEnv.getSeed());

    FMIP fMIP(originalMIP);
    OMIP oMIP(originalMIP);
    
    int xLength = originalMIP.getNumCols();
    std::vector<double> initFix(xLength,CPX_INFBOUND);
    FixPolicy::firstThetaFixing(fMIP,initFix,0.5);
    initFix.resize(fMIP.getNumCols(),CPX_INFBOUND);

    fMIP.setVarsValues(initFix);
    fMIP.solve(CLIEnv.getTimeLimit());
    std::cout<<fMIP.getObjValue()<<std::endl;
    std::vector<double> first_sol = fMIP.getSol();
    first_sol.resize(fMIP.getNumCols(),CPX_INFBOUND);
    oMIP.setVarsValues(first_sol);
    oMIP.updateBudgetConstr(fMIP.getObjValue());
    oMIP.solve(CLIEnv.getTimeLimit());
    std::cout<<oMIP.getObjValue()<<std::endl;

    /*
    switch (STATE) {
        case CPXMIP_TIME_LIM_FEAS:      // exceeded time limit, found intermediate solution
            Logger::print(WARN,"exceeded time limit, intermediate solution found.");
            break;
        case CPXMIP_TIME_LIM_INFEAS:    // exceeded time limit, no intermediate solution found
            Logger::print(WARN,"exceeded time limit, no intermediate solution found.");
            break;
        case CPXMIP_INFEASIBLE:         // proven to be infeasible
            Logger::print(ERROR,"infeasible problem.");
            break;
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
            Logger::print(ERROR,"Unhandled cplex status: %d", STATE);
            break;
    }
    */
}
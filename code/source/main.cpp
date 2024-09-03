#include "../include/FMIP.hpp"
#include "../include/OMIP.hpp"
#include "../include/FixPolicy.hpp"

int main(int argc, char* argv[]){
    std::srand(SEED);
    FMIP f_mip("2club200v15p5scn");
    OMIP o_mip("2club200v15p5scn");
    int x_length = f_mip.getNumCols() - 2 * f_mip.getNumRows();
    std::vector<double> initFix(x_length,CPX_INFBOUND);
    FixPolicy::firstThetaFixing(f_mip,initFix,0.5);
    initFix.resize(f_mip.getNumCols(),CPX_INFBOUND);
    f_mip.setVarsValues(initFix);
    f_mip.saveModel();
    f_mip.solve(100);
    std::vector<double> first_sol = f_mip.getSol();
    first_sol.resize(f_mip.getNumCols(),CPX_INFBOUND);
    o_mip.setVarsValues(first_sol);
    o_mip.updateBudgetConstr(f_mip.getObjValue());
    o_mip.saveModel();
    o_mip.solve(100);
    std::cout<<o_mip.getObjValue()<<std::endl;
    
    
    
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
    }*/
    return 0;
}
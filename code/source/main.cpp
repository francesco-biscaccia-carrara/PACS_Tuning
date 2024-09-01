#include "../include/FMIP.hpp"

int main(){
    FMIP FMIP("test");
    std::cout<<FMIP.get_num_rows()<<std::endl;
    FMIP.prepare();
    FMIP.solve(100);
    FMIP.save_model();

    /*
    switch (STATE) {
        case CPXMIP_TIME_LIM_FEAS:      // exceeded time limit, found intermediate solution
            print_state(WARN,"exceeded time limit, intermediate solution found.");
            break;
        case CPXMIP_TIME_LIM_INFEAS:    // exceeded time limit, no intermediate solution found
            print_state(WARN,"exceeded time limit, no intermediate solution found.");
            break;
        case CPXMIP_INFEASIBLE:         // proven to be infeasible
            print_state(ERROR,"infeasible problem.");
            break;
        case CPXMIP_ABORT_FEAS:         // terminated by user, found solution
            print_state(WARN,"terminated by user, found solution found.");
            break;
        case CPXMIP_ABORT_INFEAS:       // terminated by user, not found solution
            print_state(WARN,"terminated by user, no solution found.");
            break;
        case CPXMIP_OPTIMAL_TOL:        // found optimal within the tollerance
            print_state(WARN,"found optimal within the tollerance.");
            break;
        case CPXMIP_OPTIMAL:            // found optimal
            print_state(WARN,"found optimal.");
            break;
        default:                        // unhandled status
            print_state(ERROR,"Unhandled cplex status: %d", STATE);
            break;
    }*/
    return 0;
}
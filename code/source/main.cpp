#include "../include/FMIP.hpp"
#include "../include/OMIP.hpp"

int main(){

    MIP_solver mip("22433");
    mip.solve(100);

    std::vector<double> obj=mip.get_obj_function();
    std::vector<double> sol=mip.get_solution();
    std::cout<<"SOL_SIZE="<<sol.size()<<std::endl;

    double cost =0;int i=0;
    for(auto e : sol){
        std::cout<<e<<"|";
        cost += e*obj[i];
        i++;
    }
    std::cout<<std::endl;
   std::cout<<cost<<std::endl;
   std::cout<<mip.get_obj_value()<<std::endl;
   std::cout<<"COLS="<<mip.get_num_cols()<<std::endl;
   std::cout<<"ROWS="<<mip.get_num_rows()<<std::endl;
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
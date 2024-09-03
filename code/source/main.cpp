#include "../include/FMIP.hpp"
#include "../include/OMIP.hpp"
#include "../include/fixing_policy.hpp"

int main(){

    std::srand(SEED);
    FMIP f_mip("2club200v15p5scn");
    OMIP o_mip("2club200v15p5scn");
    int x_length = f_mip.get_num_cols() - 2 * f_mip.get_num_rows();
    std::vector<double> init_fixing(x_length,CPX_INFBOUND);
    fixing_policy::first_theta_var(f_mip,init_fixing,0.5);
    init_fixing.resize(f_mip.get_num_cols(),CPX_INFBOUND);
    f_mip.set_vars_value(init_fixing);
    f_mip.save_model();
    f_mip.solve(100);
    std::vector<double> first_sol = f_mip.get_solution();
    first_sol.resize(f_mip.get_num_cols(),CPX_INFBOUND);
    o_mip.set_vars_value(first_sol);
    o_mip.update_budget_constraint(f_mip.get_obj_value());
    o_mip.save_model();
    o_mip.solve(100);
    std::cout<<o_mip.get_obj_value()<<std::endl;
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
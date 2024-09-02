#include "../include/OMIP.hpp"

#define OMIP_SLACK_OBJ_COEFF 0
#define OMIP_BUD_CONST_SENSE 'L'

OMIP::OMIP(std::string file_name) : MIP_solver(file_name){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_OMIP.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const OMIP& other_solver) : MIP_solver(other_solver){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_OMIP_clone.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const MIP_solver& other_solver) : MIP_solver(other_solver){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_OMIP_clone.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}


void OMIP::save_model(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+file_name+"_OMIP.lp").c_str(), NULL);
    #endif
}


void OMIP::update_budget_constraint(double rhs){
    remove_row(get_num_rows()-1);
    add_budget_constraint(rhs);
}

std::vector<double> OMIP::get_solution(){
    std::vector<double> x_star = MIP_solver::get_solution();
    x_star.resize(get_num_cols()-2*(get_num_rows()-1));
    return x_star;
}   

void OMIP::prepare(){
    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=1;
        add_col(col,OMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=-1;
        add_col(col,OMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }

    add_budget_constraint(CPX_INFBOUND);
}

void OMIP::add_budget_constraint(double rhs){
    std::vector<double> budg_constr(get_num_cols(),0);
    int starting_point = (get_num_cols()-2*get_num_rows());
    for(int i=starting_point;i<get_num_cols();i++){
        budg_constr[i]=1;
    }
    add_row(budg_constr,OMIP_BUD_CONST_SENSE,rhs);
}

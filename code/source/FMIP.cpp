#include "../include/FMIP.hpp"

#define FMIP_SLACK_OBJ_COEFF 1
#define FMIP_VAR_OBJ_COEFF 0

FMIP::FMIP(std::string file_name) : MIP_solver(file_name){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_FMIP.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const FMIP& other_solver) : MIP_solver(other_solver){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_FMIP_clone.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const MIP_solver& other_solver) : MIP_solver(other_solver){
    prepare();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+"_FMIP_clone.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

void FMIP::save_model(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+file_name+"_FMIP.lp").c_str(), NULL);
    #endif
}

std::vector<double> FMIP::get_solution(){
    std::vector<double> x_star = MIP_solver::get_solution();
    x_star.resize(get_num_cols()-2*get_num_rows());
    return x_star;
}  

void FMIP::prepare(){
    std::vector<double> obj(get_num_cols(),FMIP_VAR_OBJ_COEFF);
    set_obj_function(obj);

    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=1;
        add_col(col,FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=-1;
        add_col(col,FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }
}
#include "../include/MIP_solver.hpp"

MIP_solver::MIP_solver(std::string file_name){

    int status;
    this->file_name = file_name;
    env = CPXopenCPLEX(&status);
    model = CPXcreateprob(env, &status, "MIP");
    if(status){
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
        print_state(ERROR,"Model not created!");
    }

    status = CPXreadcopyprob(env, model, (INST_DIR+file_name+".mps").c_str(),"MPS");
    if(status){
        print_state(ERROR,"Failed to read the problem from file! %d",status);
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
    }

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+file_name+".log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

MIP_solver::MIP_solver(const MIP_solver& other_solver){

    int status;
    this->file_name = other_solver.file_name;
    env = CPXopenCPLEX(&status);
    model = CPXcloneprob(env,other_solver.model, &status);

    if(status){
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
        print_state(ERROR,"Model not cloned!");
    }

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env, (CPLEX_LOG_DIR+file_name+"clone.log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

int MIP_solver::solve(double time_limit){

    if(time_limit < EPSILON) print_state(ERROR, "Time-limit (%10.4f) is too short!", time_limit);

    CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_MIPGap, 0);
	CPXsetdblparam(env,CPX_PARAM_TILIM,time_limit);
	
	if (int error = CPXmipopt(env,model)) print_state(ERROR, "MIPopt error! %d",error);

    return CPXgetstat(env,model);
}

double MIP_solver::get_obj_value(){
    double obj_value;
    CPXgetobjval(env,model,&obj_value);
    return obj_value;
}

std::vector<double> MIP_solver::get_solution(){
    int num_vars = CPXgetnumcols(env,model)-1;
    double* x_star = (double* ) calloc(num_vars,sizeof(double));
    if (CPXgetx(env,model, x_star, 0, num_vars)) print_state(ERROR, "CPXgetx() error!");
    std::vector<double> sol(x_star, x_star+num_vars);
    std::transform(sol.begin(), sol.end(), sol.begin(), [](double el) {
        return (el > 0.5) ? 1.0 : 0.0;
    });
    
    free(x_star);
    return sol;
}

MIP_solver::~MIP_solver(){
    CPXfreeprob(env, &model);
    CPXcloseCPLEX(&env);
}
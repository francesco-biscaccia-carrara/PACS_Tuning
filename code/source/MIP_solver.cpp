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
    this->file_name = other_solver.file_name+"_clone";
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
        if (CPXsetlogfilename(env, (CPLEX_LOG_DIR+file_name+".log").c_str(), "w") ) print_state(ERROR, "CPXsetlogfilename error!");
    #endif
}

int MIP_solver::solve(double time_limit){

    if(time_limit < EPSILON) print_state(ERROR, "Time-limit (%10.4f) is too short!", time_limit);

    CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_MIPGap, 0);
	CPXsetdblparam(env,CPX_PARAM_TILIM,time_limit);
	
	if (int error = CPXmipopt(env,model)) print_state(ERROR, "CPLEX cannot solve this problem! %d",error);

    return CPXgetstat(env,model);
}

void MIP_solver::save_model(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+file_name+".lp").c_str(), NULL);
    #endif
}

double MIP_solver::get_obj_value(){
    double obj_value;
    if(CPXgetobjval(env,model,&obj_value)) print_state(ERROR,"Unable to obtain obj value!");
    return obj_value;
}

std::vector<double> MIP_solver::get_solution(){
    int num_vars = CPXgetnumcols(env,model)-1;
    double* x_star = (double* ) calloc(num_vars,sizeof(double));
    if (CPXgetx(env,model, x_star, 0, num_vars)) print_state(ERROR, "Unable to obtain the solution!");
    std::vector<double> sol(x_star, x_star+num_vars);
    std::transform(sol.begin(), sol.end(), sol.begin(), [](double el) {
        return (el > 0.5) ? 1.0 : 0.0;
    });
    
    free(x_star);
    return sol;
}

int MIP_solver::get_num_cols() {return CPXgetnumcols(env,model);}
int MIP_solver::get_num_rows() {return CPXgetnumrows(env,model);}
int MIP_solver::get_num_vars() {return CPXgetnumcols(env,model)-1;}

std::vector<double> MIP_solver::get_obj_function(){
    int num_vars = CPXgetnumcols(env,model)-1;
    double* obj_f = (double* ) calloc(num_vars,sizeof(double));
    if(CPXgetobj(env, model, obj_f, 0, num_vars)) print_state(ERROR, "Unable to get obj. coefficients!");
    std::vector<double> obj(obj_f, obj_f+num_vars);
    free(obj_f);
    return obj;
}

void MIP_solver::set_obj_function(std::vector<double> new_obj){
    int num_cols = CPXgetnumcols(env,model);
    if(new_obj.size() != (num_cols-1)) print_state(ERROR,"No suitable obj_function coefficients");

    int* indices = (int*) malloc(num_cols*sizeof(int));
    for(int i=0;i<num_cols;i++) indices[i]=i;
    if(CPXchgobj(env, model, num_cols, indices, &new_obj[0])) print_state(ERROR,"obj_function not changed");
    free(indices);
}

void MIP_solver::add_col(std::vector<double> new_col, double obj_coef, double ub, double lb, std::string name){
    int num_row = CPXgetnumrows(env,model);

    if(new_col.size() != num_row) print_state(ERROR,"Wrong column size!");

    char** cname = (char**) calloc(1,sizeof(char*));
    char col_name[name.length()];
    strcpy(col_name,name.c_str());
    cname[0]=col_name;
    int* indices = (int*) malloc(num_row*sizeof(int));
    double* values = (double*) malloc(num_row*sizeof(int));
    int start_index = 0;
    int nnz =0;
    for(int i=0;i<num_row;i++){
        if(new_col[i]!=0){
            indices[nnz]=i;
            values[nnz]=new_col[i];
            nnz++;
        }
    }

    if(CPXaddcols(env, model, 1, nnz, &obj_coef, &start_index , indices, values, &lb, &ub, &cname[0])) print_state(ERROR,"No Column added!");
    free(cname);
    free(indices);
    free(values);
}

void MIP_solver::add_row(std::vector<double> new_row, char sense, double rhs){
    int num_cols = CPXgetnumcols(env,model);

    if(new_row.size() != num_cols) print_state(ERROR,"Wrong row size!");
    
    int* indices = (int*) malloc(num_cols*sizeof(int));
    double* values = (double*) malloc(num_cols*sizeof(int));
    int start_index = 0;
    int nnz =0;
    for(int i=0;i<num_cols;i++){
        if(new_row[i]!=0){
            indices[nnz]=i;
            values[nnz]=new_row[i];
            nnz++;
        }
    }

    if(CPXaddrows(env, model, 0, 1, nnz, &rhs, &sense, &start_index , indices, values, NULL,NULL)) print_state(ERROR,"No Column added!");
    free(indices);
    free(values);

}

void MIP_solver::set_var_value(int index, double val){
    char bound = 'B'; //Both
    CPXchgbds(env, model, 1, &index, &bound, &val);
}

MIP_solver::~MIP_solver(){
    CPXfreeprob(env, &model);
    CPXcloseCPLEX(&env);
}
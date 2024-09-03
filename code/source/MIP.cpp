#include "../include/MIP.hpp"


MIP:: MIP(const std::string fileName){

    int status;
    this->fileName = fileName;
    env = CPXopenCPLEX(&status);
    model = CPXcreateprob(env, &status, "MIP");
    if(status){
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
        Logger::print(ERROR,"Model not created!");
    }

    status = CPXreadcopyprob(env, model, (INST_DIR+fileName+".mps").c_str(),"MPS");
    if(status){
        Logger::print(ERROR,"Failed to read the problem from file! %d",status);
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
    }

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+".log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}


MIP::MIP(const MIP& otherMIP){

    int status;
    this->fileName = otherMIP.fileName+"_clone";
    env = CPXopenCPLEX(&status);
    model = CPXcloneprob(env,otherMIP.model, &status);

    if(status){
        CPXfreeprob(env, &model);
        CPXcloseCPLEX(&env);
        Logger::print(ERROR,"Model not cloned!");
    }

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env, (CPLEX_LOG_DIR+fileName+".log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}


void MIP::setNumCores(const int numCores){
    if(CPXsetintparam(env, CPX_PARAM_THREADS, numCores)) Logger::print(ERROR, "CPX_PARAM_THREADS not cahanged!");
}


int MIP::solve(const double timeLimit){

    if(timeLimit < EPSILON) Logger::print(ERROR, "Time-limit (%10.4f) is too short!", timeLimit);

    CPXsetdblparam(env, CPXPARAM_MIP_Tolerances_MIPGap, 0);
	CPXsetdblparam(env,CPX_PARAM_TILIM,timeLimit);
	
	if (int error = CPXmipopt(env,model)) Logger::print(ERROR, "CPLEX cannot solve this problem! %d",error);

    return CPXgetstat(env,model);
}


void MIP::saveModel(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+".lp").c_str(), NULL);
    #endif
}


double MIP::getObjValue(){
    double objValue;
    if(CPXgetobjval(env,model,&objValue)) Logger::print(ERROR,"Unable to obtain obj value!");
    return objValue;
}


std::vector<double> MIP::getObjFunction(){
    int numCols = CPXgetnumcols(env,model);
    double* objFun = (double* ) calloc(numCols,sizeof(double));
    if(CPXgetobj(env, model, objFun, 0, numCols-1)) Logger::print(ERROR, "Unable to get obj. coefficients!");
    std::vector<double> obj(objFun, objFun+numCols);
    free(objFun);
    return obj;
}


void MIP::setObjFunction(const std::vector<double>& newObj){
    int numCols = CPXgetnumcols(env,model);
    if(newObj.size() != numCols) Logger::print(ERROR,"No suitable obj_function coefficients");

    int* indices = (int*) malloc(numCols*sizeof(int));
    for(int i=0;i<numCols;i++) indices[i]=i;
    if(CPXchgobj(env, model, numCols, indices, &newObj[0])) Logger::print(ERROR,"obj_function not changed");
    free(indices);
}


std::vector<double> MIP::getSol(){
    int numCols = CPXgetnumcols(env,model);
    double* xStar = (double* ) calloc(numCols,sizeof(double));
    if (CPXgetx(env,model, xStar, 0, numCols-1)) Logger::print(ERROR, "Unable to obtain the solution!");
    std::vector<double> sol(xStar, xStar+numCols);
    free(xStar);
    return sol;
}


int MIP::getNumCols() {return CPXgetnumcols(env,model);} // num cols = num var


int MIP::getNumRows() {return CPXgetnumrows(env,model);}


void MIP::addCol(const std::vector<double>& newCol, const double objCoef,const double lb, const double ub, const std::string name){
    int numRow = CPXgetnumrows(env,model);

    if(newCol.size() != numRow) Logger::print(ERROR,"Wrong column size!");

    char** cname = (char**) calloc(1,sizeof(char*));
    char colName[name.length()];
    strcpy(colName,name.c_str());
    cname[0]=colName;
    int* indices = (int*) malloc(numRow*sizeof(int));
    double* values = (double*) malloc(numRow*sizeof(double));
    int start = 0, nnz = 0;
    for(int i=0;i<numRow;i++){
        if(newCol[i]!=0){
            indices[nnz]=i;
            values[nnz]=newCol[i];
            nnz++;
        }
    }

    if(CPXaddcols(env, model, 1, nnz, &objCoef, &start , indices, values, &lb, &ub, &cname[0])) Logger::print(ERROR,"No Column added!");
    free(cname);
    free(indices);
    free(values);
}


void MIP::addRow(const std::vector<double>& newRow,const char sense,const double rhs){
    int numCols = CPXgetnumcols(env,model);

    if(newRow.size() != numCols) Logger::print(ERROR,"Wrong row size!");
    
    int* indices = (int*) malloc(numCols*sizeof(int));
    double* values = (double*) malloc(numCols*sizeof(double));
    int start = 0,nnz =0;
    for(int i=0;i<numCols;i++){
        if(newRow[i]!=0){
            indices[nnz]=i;
            values[nnz]=newRow[i];
            nnz++;
        }
    }

    if(CPXaddrows(env, model, 0, 1, nnz, &rhs, &sense, &start , indices, values, NULL,NULL)) Logger::print(ERROR,"No Column added!");
    free(indices);
    free(values);

}


void MIP::removeRow(const int index){
    if(CPXdelrows (env, model, index, index)) Logger::print(ERROR,"Row not removed!");
}


void MIP::removeCol(const int index){
    if(CPXdelcols (env, model, index, index)) Logger::print(ERROR,"Row not removed!");
}


std::pair<double,double> MIP::getVarBounds(const int index){
    double lb = CPX_INFBOUND, ub = CPX_INFBOUND;
    if(CPXgetlb(env, model, &lb, index, index)) Logger::print(ERROR,"Unable to get the var lower_bound!");
    if(CPXgetub(env, model, &ub, index, index)) Logger::print(ERROR,"Unable to get the var lower_bound!");
    return std::make_pair(lb, ub);
}


void MIP::setVarValues(const int index, const double val){
    char bound = 'B'; //Both
    CPXchgbds(env, model, 1, &index, &bound, &val);
}


void MIP::setVarsValues(const std::vector<double>& values){
    int numCols = CPXgetnumcols(env,model);
    
    if(values.size() != numCols) Logger::print(ERROR,"Wrong values size!");
    for(int i=0;i<numCols;i++){
        if(values[i] < CPX_INFBOUND){
            setVarValues(i,values[i]);
        }
    }
}


MIP::~MIP(){
    CPXfreeprob(env, &model);
    CPXcloseCPLEX(&env);
}
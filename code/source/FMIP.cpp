#include "../include/FMIP.hpp"

#define FMIP_SLACK_OBJ_COEFF 1
#define FMIP_VAR_OBJ_COEFF 0

FMIP::FMIP(const std::string fileName) : MIP(fileName){
    setup();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_FMIP.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const FMIP& otherFMIP) : MIP(otherFMIP){

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_FMIP_clone.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const MIP& otherMIP) : MIP(otherMIP){
    setup();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_FMIP_clone.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

void FMIP::saveModel(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+"_FMIP.lp").c_str(), NULL);
    #endif
}

std::vector<double> FMIP::getSol(){
    std::vector<double> x_star = MIP::getSol();
    x_star.resize(getNumCols()-2*getNumRows());
    return x_star;
}  

void FMIP::setup(){
    std::vector<double> obj(getNumCols(),FMIP_VAR_OBJ_COEFF);
    setObjFunction(obj);

    for(int i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0);
        col[i]=1;
        addCol(col,FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0);
        col[i]=-1;
        addCol(col,FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }
}
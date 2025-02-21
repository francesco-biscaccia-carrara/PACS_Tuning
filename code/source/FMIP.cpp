#include "../include/FMIP.hpp"


FMIP::FMIP(const std::string fileName) : MIP(fileName){
    setup();

    #if ACS_VERBOSE == 1
        if (setLogFileName(fileName+"_FMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const FMIP& otherFMIP) : MIP(otherFMIP){

    #if ACS_VERBOSE == 1
        if (setLogFileName(fileName+"_clone_FMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

FMIP::FMIP(const MIP& otherMIP) : MIP(otherMIP){
    setup();

    #if ACS_VERBOSE == 1
        if (setLogFileName(fileName+"_clone_FMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}


void FMIP::saveModel(){
    #if ACS_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+"_FMIP.lp").c_str(), NULL);
    #endif
}

std::vector<double> FMIP::getSol(){
    std::vector<double> x_star = MIP::getSol();
    x_star.resize(getNumCols()-2*getNumRows());
    return x_star;
}  

void FMIP::setup(){
    std::vector<double> obj(getNumCols(),FMIP::FMIP_VAR_OBJ_COEFF);
    setObjFunction(obj);

    for(size_t i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0);
        col[i]=1;
        addCol(col,FMIP::FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(size_t i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0);
        col[i]=-1;
        addCol(col,FMIP::FMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }
}
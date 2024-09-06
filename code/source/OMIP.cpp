#include "../include/OMIP.hpp"


OMIP::OMIP(std::string fileName) : MIP(fileName){
    setup();

    #if MH_VERBOSE == 1
        if (setLogFileName(fileName+"_OMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const OMIP& otherOMIP) : MIP(otherOMIP){

    #if MH_VERBOSE == 1
        if (setLogFileName(fileName+"_clone_OMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const MIP& otherMIP) : MIP(otherMIP){
    setup();

    #if MH_VERBOSE == 1
        if (setLogFileName(fileName+"_clone_OMIP")) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}


void OMIP::saveModel(){
    #if MH_VERBOSE == 1
        CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+"_OMIP.lp").c_str(), NULL);
    #endif
}


void OMIP::updateBudgetConstr(double rhs){
    removeRow(getNumRows()-1);
    addBudgetConstr(rhs);
}

std::vector<double> OMIP::getSol(){
    std::vector<double> xStar = MIP::getSol();
    xStar.resize(getNumCols()-2*(getNumRows()-1));
    return xStar;
}   

void OMIP::setup(){
    for(int i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0.);
        col[i]=1;
        addCol(col,OMIP::OMIP_SLACK_OBJ_COEFF,0.,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0.);
        col[i]=-1;
        addCol(col,OMIP::OMIP_SLACK_OBJ_COEFF,0.,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }

    addBudgetConstr(CPX_INFBOUND);
}

void OMIP::addBudgetConstr(double rhs){
    std::vector<double> budConstr(getNumCols(),0.);
    int start = (getNumCols()-2*getNumRows());
    for(int i=start;i<getNumCols();i++){
        budConstr[i]=1.;
    }
    addRow(budConstr,OMIP::OMIP_BUD_CONST_SENSE,rhs);
}

#include "../include/OMIP.hpp"

#define OMIP_SLACK_OBJ_COEFF 0
#define OMIP_BUD_CONST_SENSE 'L'

OMIP::OMIP(std::string fileName) : MIP(fileName){
    setup();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_OMIP.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const OMIP& otherOMIP) : MIP(otherOMIP){

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_OMIP_clone.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
    #endif
}

OMIP::OMIP(const MIP& otherMIP) : MIP(otherMIP){
    setup();

    #if MH_VERBOSE == 1
        CPXsetdblparam(env, CPX_PARAM_SCRIND, CPX_OFF);
	    CPXsetintparam(env, CPX_PARAM_CLONELOG, -1);
        if (CPXsetlogfilename(env,  (CPLEX_LOG_DIR+fileName+"_OMIP_clone.log").c_str(), "w") ) Logger::print(ERROR, "CPXsetlogfilename error!");
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
        std::vector<double> col(getNumRows(),0);
        col[i]=1;
        addCol(col,OMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<getNumRows();i++){
        std::vector<double> col(getNumRows(),0);
        col[i]=-1;
        addCol(col,OMIP_SLACK_OBJ_COEFF,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }

    addBudgetConstr(CPX_INFBOUND);
}

void OMIP::addBudgetConstr(double rhs){
    std::vector<double> budConstr(getNumCols(),0);
    int start = (getNumCols()-2*getNumRows());
    for(int i=start;i<getNumCols();i++){
        budConstr[i]=1;
    }
    addRow(budConstr,OMIP_BUD_CONST_SENSE,rhs);
}

#ifndef MIP_SOL_H
#define MIP_SOL_H

#include "Utils.hpp"
#include <cplex.h>

using namespace Utils;
using namespace Logger;


class MIP{

    public:
        MIP(const std::string fileName);
        MIP(const MIP& otherMIP);
        MIP& operator=(const MIP&) = delete;
        
        MIP& setNumCores(const int numCores);
        int solve(const double timeLimit);
        int solveRelaxation(const double timeLimit);
        
        
        double getObjValue();
        std::vector<double> getObjFunction();
        MIP& setObjFunction(const std::vector<double>& newObj);

        std::vector<double> getSol();

        inline int getNumCols(){return CPXgetnumcols(env,model);}; // num cols = num var
        int getNumRows(){{return CPXgetnumrows(env,model);}} // num cols = num constr

        MIP& addCol(const std::vector<double>& newCol, const double objCoef,const double lb, const double ub, const std::string name);
        MIP& addRow(const std::vector<double>& newRow,const char sense,const double rhs);
        MIP& removeRow(const int index);
        MIP& removeCol(const int index);

        
        std::pair<double,double> getVarBounds(const int index);
        char getVarType(const int index);
        MIP& changeVarType(const int index,const char type);
        MIP& setVarValues(const int index, const double val);
        MIP& setVarsValues(const std::vector<double>& values);

        #if ACS_VERBOSE == 1
        MIP& saveModel();
        #endif

        ~MIP();

    protected:
        CPXLPptr model; 
        CPXENVptr env;
        std::string fileName;
        const std::string CPLEX_LOG_DIR ="../log/cplex_out/log/";
        const std::string MIP_LOG_DIR = "../log/cplex_out/mip/";
        const std::string INST_DIR = "../data/";
        
        int setLogFileName(std::string logFileName);

    private:
        std::vector<std::pair<int,char>> restoreVarType;
        const double DUAL_PRIM_GAP_TOL = 1e-4;
        const double MIP_GAP_TOL = 0.0;

        MIP& changeProbType(const int type);
       

};


#endif
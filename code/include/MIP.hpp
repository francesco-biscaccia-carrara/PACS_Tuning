#ifndef MIP_SOL_H
#define MIP_SOL_H

#include "Utils.hpp"
#include <cplex.h>


class MIP{

    public:
        explicit MIP(const std::string fileName);
        explicit MIP(const MIP& otherMIP);
        
        void setNumCores(const int numCores);
        int solve(const double timeLimit);
        int solveRelaxation(const double timeLimit);
        void saveModel();
        
        double getObjValue();
        std::vector<double> getObjFunction();
        void setObjFunction(const std::vector<double>& newObj);

        std::vector<double> getSol();

        int getNumCols();
        int getNumRows();

        void addCol(const std::vector<double>& newCol, const double objCoef,const double lb, const double ub, const std::string name);
        void addRow(const std::vector<double>& newRow,const char sense,const double rhs);
        void removeRow(const int index);
        void removeCol(const int index);

        
        std::pair<double,double> getVarBounds(const int index);
        char getVarType(const int index);
        void changeVarType(const int index,const char type);
        void setVarValues(const int index, const double val);
        void setVarsValues(const std::vector<double>& values);

        ~MIP();

    protected:
        CPXLPptr model; 
        CPXENVptr env;
        std::string fileName;
        const std::string CPLEX_LOG_DIR ="../log/cplex_out/log/";
        const std::string MIP_LOG_DIR = "../log/cplex_out/mip/";
        const std::string INST_DIR = "../instances/";
        
        int setLogFileName(std::string logFileName);

    private:
        std::vector<std::pair<int,char>> restoreVarType;
        const double DUAL_PRIM_GAP_TOL = 1e-4;
        const double MIP_GAP_TOL = 0.0;

        void changeProbType(const int type);
       

};


#endif
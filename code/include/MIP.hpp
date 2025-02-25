#ifndef MIP_SOL_H
#define MIP_SOL_H

#include "Utils.hpp"
#include <cplex.h>

using namespace Utils;

#define CPLEX_LOG_DIR "../log/cplex_out/log/"
#define MIP_LOG_DIR "../log/cplex_out/mip/"
#define INST_DIR "../data/"

#define MIP_DUAL_PRIM_GAP_TOL 1e-4
#define MIP_GAP_TOL 0.0

class MIP {

public:
	MIP(const std::string fileName);
	MIP(const MIP& otherMIP);
	MIP& operator=(const MIP&) = delete;

	MIP& setNumCores(const int numCores);
	int	 solve(const double timeLimit);
	int	 solveRelaxation(const double timeLimit);

	double				getObjValue();
	std::vector<double> getObjFunction();
	MIP&				setObjFunction(const std::vector<double>& newObj);

	std::vector<double> getSol();

	inline int getNumCols() { return CPXgetnumcols(env, model); }; // num cols = num var
	inline int getNumRows() { return CPXgetnumrows(env, model); }; // num rows = num constr

	MIP& addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name);
	MIP& addRow(const std::vector<double>& newRow, const char sense, const double rhs);
	MIP& removeRow(const int index);
	MIP& removeCol(const int index);

	VarBounds getVarBounds(const int index);
	char	  getVarType(const int index);
	MIP&	  changeVarType(const int index, const char type);
	MIP&	  setVarValues(const int index, const double val);
	MIP&	  setVarsValues(const std::vector<double>& values);

#if ACS_VERBOSE == DEBUG
	inline void saveModel() { CPXwriteprob(env, model, (MIP_LOG_DIR + fileName + ".lp").c_str(), NULL); };
#endif

	~MIP();

protected:
	CPXLPptr	model;
	CPXENVptr	env;
	std::string fileName;

#if ACS_VERBOSE == DEBUG
	inline int setLogFileName(std::string logFileName) { return CPXsetlogfilename(env, (CPLEX_LOG_DIR + logFileName + ".log").c_str(), "w"); };
#endif

private:
	std::vector<char> restoreVarType;

	MIP& changeProbType(const int type);
};

#endif
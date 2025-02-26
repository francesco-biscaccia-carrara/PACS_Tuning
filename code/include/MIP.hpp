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

	[[nodiscard]]
	double getObjValue();
	[[nodiscard]]
	std::vector<double> getObjFunction();
	MIP&				setObjFunction(const std::vector<double>& newObj);

	[[nodiscard]]
	std::vector<double> getSol();

	[[nodiscard]]
	inline int getNumCols() { return CPXgetnumcols(env, model); }; // num cols = num var
	[[nodiscard]]
	inline int getNumRows() { return CPXgetnumrows(env, model); }; // num rows = num constr

	MIP& addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name);
	MIP& addRow(const std::vector<double>& newRow, const char sense, const double rhs);
	MIP& removeRow(const int index);
	MIP& removeCol(const int index);

	VarBounds getVarBounds(const int index);

	[[nodiscard]]
	char getVarType(const int index);
	MIP& changeVarType(const int index, const char type);
	MIP& setVarValues(const int index, const double val);
	MIP& setVarsValues(const std::vector<double>& values);

#if ACS_VERBOSE == DEBUG
	inline MIP& saveModel() {
		CPXwriteprob(env, model, (MIP_LOG_DIR + fileName + "_" + id + ".lp").c_str(), NULL);
		return *this;
	};
	inline MIP& saveLog() {
		CPXsetlogfilename(env, (CPLEX_LOG_DIR + fileName + "_" + id + ".log").c_str(), "w");
		return *this;
	}
#endif

	~MIP();

protected:
	CPXLPptr  model;
	CPXENVptr env;

#if ACS_VERBOSE == DEBUG
	std::string fileName;
	std::string id;
#endif

private:
	std::vector<char> restoreVarType;

	MIP& changeProbType(const int type);
};

#endif
#ifndef MIP_SOL_H
#define MIP_SOL_H

#include "Utils.hpp"
#include <cplex.h>

using namespace Utils;

#define CPLEX_LOG_DIR "../log/cplex_out/log/"
#define MIP_LOG_DIR "../log/cplex_out/mip/"
#define INST_DIR "../data/"
#define CPLEX_CORE 1

#define MIP_DUAL_PRIM_GAP_TOL 1e-4
#define MIP_GAP_TOL 0.0

class MIPException : public std::runtime_error {

public:
	enum class ExceptionType {
		General,
		ModelCreation,
		OutOfBound,
		MIPOptimizationError,
		LPOptimizationError,
		WrongTimeLimit,
		FileNotFound,
		InputSizeError,
		_count // Helper for array size
	};

	explicit MIPException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

private:
	static constexpr std::array<const char*, static_cast<size_t>(ExceptionType::_count)> typeNames = {
		"_general-ex_",
		"ModelCreation",
		"OutOfBounds",
		"MIPOptimizationError",
		"LPOptimizationError",
		"WrongTimeLimit",
		"FileNotFound",
		"InputSizeError"
	};

	static std::string formatMessage(ExceptionType type, const std::string& message) {
		return "MIPException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
	}
};

class MIP {

public:
	MIP(const std::string fileName);
	MIP(const MIP& otherMIP);
	MIP& operator=(const MIP&) = delete;

	MIP& setNumCores(const int numCores);
	size_t getNumNonZeros();

	int	 solve(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);
	MIP& addMIPStart(const std::vector<double>& MIPStart, bool CPLEXCheck = false);

	[[nodiscard]]
	double getObjValue();
	[[nodiscard]]
	std::vector<double> getObjFunction();
	MIP&				setObjFunction(const std::vector<double>& newObj);

	[[nodiscard]]
	std::vector<double> getSol();

	[[nodiscard]]
	virtual int getMIPNumVars() noexcept { return CPXgetnumcols(env, model); }
	[[nodiscard]]
	inline int getNumCols() noexcept { return CPXgetnumcols(env, model); } // num cols = num var
	[[nodiscard]]
	inline int getNumRows() noexcept { return CPXgetnumrows(env, model); } // num rows = num constr

	MIP& addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name);
	MIP& addCol(const size_t index, const double value, const double objCoef, const double lb, const double ub, const std::string name);
	MIP& addRow(const std::vector<double>& newRow, const char sense, const double rhs);
	MIP& removeRow(const int index);
	MIP& removeCol(const int index);

	VarBounds getVarBounds(const int index);

	[[nodiscard]]
	char getVarType(const int index);
	MIP& changeVarType(const int index, const char type);
	MIP& setVarValue(const int index, const double val);
	MIP& setVarsValues(const std::vector<double>& values);

	[[nodiscard]]
	bool checkFeasibility(const std::vector<double>& sol);

#if ACS_VERBOSE == DEBUG
	inline MIP& saveModel() noexcept {
		CPXwriteprob(env, model, (MIP_LOG_DIR + fileName + "_" + id + ".lp").c_str(), NULL);
		return *this;
	}
	inline MIP& saveLog() noexcept {
		CPXsetlogfilename(env, (CPLEX_LOG_DIR + fileName + "_" + id + ".log").c_str(), "w");
		return *this;
	}
	std::string getId() { return id; }
#endif

	~MIP() noexcept;

protected:
	CPXLPptr  model;
	CPXENVptr env;

#if ACS_VERBOSE == DEBUG
	std::string fileName;
	std::string id;
#endif
};

#endif
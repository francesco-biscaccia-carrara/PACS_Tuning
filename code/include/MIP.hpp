/**
 * @file MIP_Sol.hpp
 * @brief Header file for Mixed Integer Programming (MIP) solution handling
 *
 * This header defines utilities for managing Mixed Integer Programming problems 
 * using CPLEX optimization library, including exception handling, solution 
 * representation, and MIP problem manipulation.
 *
 * @note Requires CPLEX library and Utils.hpp
 * @author Francesco Biscaccia Carrara
 * @version v1.0.4
 * @since 03/31/2025
 */

#ifndef MIP_SOL_H
#define MIP_SOL_H

#include <cplex.h>

#include "Utils.hpp"
using namespace Utils;

#define CPLEX_LOG_DIR "../log/cplex_out/log/"
#define MIP_LOG_DIR "../log/cplex_out/mip/"
#define INST_DIR "../data/"
#define CPLEX_CORE 1

#define MIP_DUAL_PRIM_GAP_TOL 1e-4
/// FIXED: Bug #5860f1916463f69833a7cb9170845d492fabee8f –- Error in solution quality due to excessively tight tolerance.
#define MIP_GAP_TOL 1e-4
/// FIXED: Bug #62f2110b0d2547498c59d7c19c3490ac15330119 –- Error in solution quality due to excessively tight tolerance.
#define MIP_INT_TOL 1e-4
#define MIP_SIMPLEX_FEAS_TOL 1e-4

/**
 * @struct VarBounds
 * @brief Represents the lower and upper bounds of a variable
 */
struct VarBounds {
	double lowerBound; ///< Lower bound of the variable
    double upperBound; ///< Upper bound of the variable
};

/**
 * @struct Solution
 * @brief Represents an optimization solution
 */
struct Solution {
	std::vector<double> sol;    ///< Solution vector of variable values
    double slackSum;            ///< Sum of slack variables
    double oMIPCost;            ///< Objective cost of the MIP solution
};

/**
 * @class MIPException
 * @brief Custom exception class for MIP-related errors
 *
 * Provides detailed exception types for various MIP processing scenarios
 */
class MIPException : public std::runtime_error {

public:
	/**
     * @enum ExceptionType
     * @brief Enumeration of possible MIP-related exception types
     */
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

	/**
     * @brief Constructs a MIPException with a specific type and message
     * @param type The type of exception
     * @param message Detailed error message
     */
	explicit MIPException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

private:
 	/// Static array of exception type names
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

	/**
     * @brief Formats the exception message with type and details
     * @param type The type of exception
     * @param message Detailed error message
     * @return Formatted error message string
     */
	static std::string formatMessage(ExceptionType type, const std::string& message) {
		return "MIPException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
	}
};

/**
 * @class MIP
 * @brief Main class for handling Mixed Integer Programming problems
 *
 * Provides methods for creating, modifying, and solving MIP problems using CPLEX
 */
class MIP {

public:
	/**
     * @brief Constructs a MIP problem from a file
     * @param fileName Path to the problem file
     */
	MIP(const std::string fileName);

	 /**
     * @brief Copy constructor
     * @param otherMIP Another MIP object to copy
     */
	MIP(const MIP& otherMIP);

	 /// Explicitly delete copy assignment operator
	MIP& operator=(const MIP&) = delete;

	 /**
     * @brief Sets the number of cores to use for solving
     * @param numCores Number of CPU cores
     * @return Reference to the current MIP object
     */
	MIP& setNumCores(const int numCores);

	/**
     * @brief Get the number of non-zero elements in the problem
     * @return Number of non-zero elements
     */
	size_t getNumNonZeros();

	/**
     * @brief Solve the MIP problem
     * @param timeLimit Maximum solving time (default: infinite)
     * @param detTimeLimit Deterministic time limit (default: infinite)
     * @return Solving status code
     */
	int	 solve(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);

	/**
     * @brief Add a MIP start solution
     * @param MIPStart Vector of initial solution values
     * @param CPLEXCheck Whether to perform CPLEX-specific checks
     * @return Reference to the current MIP object
     */
	MIP& addMIPStart(const std::vector<double>& MIPStart, bool CPLEXCheck = false);

	/**
     * @brief Get the objective value of the solution
     * @return Objective value
     */
	[[nodiscard]]
	double getObjValue();

	/**
     * @brief Get the objective function coefficients
     * @return Vector of objective function coefficients
     */
	[[nodiscard]]
	std::vector<double> getObjFunction();

	/**
     * @brief Set a new objective function
     * @param newObj Vector of new objective function coefficients
     * @return Reference to the current MIP object
     */
	MIP&				setObjFunction(const std::vector<double>& newObj);

	/**
     * @brief Get the solution vector
     * @return Vector of solution variable values
     */
	[[nodiscard]]
	std::vector<double> getSol();

	/**
     * @brief Get the number of MIP variables
     * @return Number of variables
     */
	[[nodiscard]]
	virtual int getMIPNumVars() noexcept { return CPXgetnumcols(env, model); }

	/**
     * @brief Get the number of columns (variables)
     * @return Number of columns
     */
	[[nodiscard]]
	inline int getNumCols() noexcept { return CPXgetnumcols(env, model); }
	
	/**
     * @brief Get the number of rows (constraints)
     * @return Number of rows
     */
	[[nodiscard]]
	inline int getNumRows() noexcept { return CPXgetnumrows(env, model); }

	/**
     * @brief Add a new column (variable) to the problem
     * @param newCol Vector representing the column
     * @param objCoef Objective function coefficient
     * @param lb Lower bound
     * @param ub Upper bound
     * @param name Variable name
     * @return Reference to the current MIP object
     */
	MIP& addCol(const std::vector<double>& newCol, const double objCoef, const double lb, const double ub, const std::string name);

	/**
     * @brief Add a new column at a specific index
     * @param index Column index
     * @param value Column value
     * @param objCoef Objective function coefficient
     * @param lb Lower bound
     * @param ub Upper bound
     * @param name Variable name
     * @return Reference to the current MIP object
     */
	MIP& addCol(const size_t index, const double value, const double objCoef, const double lb, const double ub, const std::string name);

	/**
     * @brief Add a new row (constraint) to the problem
     * @param newRow Vector representing the row
     * @param sense Constraint sense (<=, =, >=)
     * @param rhs Right-hand side of the constraint
     * @return Reference to the current MIP object
     */
	MIP& addRow(const std::vector<double>& newRow, const char sense, const double rhs);

	/**
     * @brief Remove a row at a specific index
     * @param index Row index to remove
     * @return Reference to the current MIP object
     */
	MIP& removeRow(const int index);

	/**
     * @brief Remove a column at a specific index
     * @param index Column index to remove
     * @return Reference to the current MIP object
     */
	MIP& removeCol(const int index);

	/**
     * @brief Get the bounds of a variable
     * @param index Variable index
     * @return VarBounds structure with lower and upper bounds
     */
	VarBounds getVarBounds(const int index);

	/**
     * @brief Set the value of a specific variable
     * @param index Variable index
     * @param val Variable value
     * @return Reference to the current MIP object
     */
	MIP& setVarValue(const int index, const double val);

	/**
     * @brief Set values for multiple variables
     * @param values Vector of variable values
     * @return Reference to the current MIP object
     */
	MIP& setVarsValues(const std::vector<double>& values);

	/**
     * @brief Check the feasibility of a solution
     * @param sol Solution vector to check
     * @return True if solution is feasible, false otherwise
     */
	[[nodiscard]]
	bool checkFeasibility(const std::vector<double>& sol);

// Debug-specific methods
#if ACS_VERBOSE == DEBUG
	/**
     * @brief Save the current model to a file (debug mode only)
     * @return Reference to the current MIP object
     */
	inline MIP& saveModel() noexcept {
		CPXwriteprob(env, model, (MIP_LOG_DIR + fileName + "_" + id + ".lp").c_str(), NULL);
		return *this;
	}

	/**
     * @brief Save the CPLEX log to a file (debug mode only)
     * @return Reference to the current MIP object
     */
	inline MIP& saveLog() noexcept {
		CPXsetlogfilename(env, (CPLEX_LOG_DIR + fileName + "_" + id + ".log").c_str(), "w");
		return *this;
	}

	/**
     * @brief Get the unique identifier of the MIP instance
     * @return Identifier string
     */
	std::string getId() { return id; }
#endif

	/**
     * @brief Destructor for the MIP class
     */
	~MIP() noexcept;

protected:
	CPXLPptr model;   ///< CPLEX LP problem pointer
    CPXENVptr env;    ///< CPLEX environment pointer

// Debug-specific methods
#if ACS_VERBOSE == DEBUG
	std::string fileName;  ///< Name of the input file
	std::string id;        ///< Unique identifier for the MIP instance
#endif
};

#endif
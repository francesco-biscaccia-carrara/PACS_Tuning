/**
 * @file RlxFMIP.hpp
 * @brief Relaxed Feasible Mixed Integer Programming class that extends FMIP.
 *
 * This class provides functionality for handling relaxed versions of Feasible Mixed
 * Integer Programming problems. It specializes in solving relaxed variants of FMIP
 * problems by manipulating variable types and problem characteristics. The class
 * enables switching between different relaxation strategies and maintains the ability
 * to restore original problem formulations.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.11
 * @since 15/09/2025
 */

#ifndef RlxFMIP_H
#define RlxFMIP_H

#include "FMIP.hpp"
using namespace Utils;

/**
 * @class RlxFMIP
 * @brief A derived class for Relaxed Feasible Mixed Integer Programming problems.
 *
 * RlxFMIP extends the FMIP class to provide specialized functionality for
 * relaxing and solving feasibility-focused mixed integer programs. This class
 * offers methods to modify variable types, solve relaxed versions of problems,
 * and manage the restoration of original problem formulations.
 */
class RlxFMIP : public FMIP {

public:
	/**
	 * @brief Constructs a RlxFMIP object from a file.
	 *
	 * @param fileName The path to the file containing the MIP problem data.
	 * @throws std::runtime_error If the file cannot be opened or contains invalid data.
	 */
	RlxFMIP(std::string fileName);

	/**
	 * @brief Deleted copy constructor.
	 *
	 * Copy construction is not supported for RlxFMIP objects to prevent
	 * potential issues with deep copying relaxation-specific data structures.
	 *
	 * @param otherRlxFMIP The RlxFMIP object to copy.
	 */
	RlxFMIP(const RlxFMIP& otherRlxFMIP) = delete;

	/**
	 * @brief Deleted conversion constructor from MIP.
	 *
	 * Conversion from base MIP objects to RlxFMIP is not supported
	 * to ensure proper initialization of relaxation-specific data.
	 *
	 * @param otherMIP The MIP object to convert from.
	 */
	RlxFMIP(const MIP& otherMIP) = delete;

	/**
	 * @brief Deleted assignment operator from MIP.
	 *
	 * Assignment from MIP to RlxFMIP is not supported to prevent object slicing
	 * and maintain class invariants.
	 *
	 * @param other The MIP object to assign from.
	 * @return Reference to this RlxFMIP object.
	 */
	RlxFMIP& operator=(const MIP&) = delete;

	/**
	 * @brief Deleted assignment operator.
	 *
	 * Assignment between RlxFMIP objects is not supported in the current implementation
	 * to prevent potential resource management issues.
	 *
	 * @param other The RlxFMIP object to assign from.
	 * @return Reference to this RlxFMIP object.
	 */
	RlxFMIP& operator=(const RlxFMIP&) = delete;

	/**
	 * @brief Solves the MIP problem with specified time limits.
	 *
	 * This method attempts to solve the Mixed Integer Programming problem
	 * using the specified time limits. It may employ specialized relaxation
	 * techniques specific to the RlxFMIP implementation.
	 *
	 * @param timeLimit Maximum wall clock time for the solution process (default: CPX_INFBOUND).
	 * @param detTimeLimit Maximum deterministic time for solution (default: CPX_INFBOUND).
	 * @return Status code indicating the result of the solution process.
	 */
	int solve(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);

	/**
	 * @brief Solves the relaxed version of the MIP problem.
	 *
	 * This method solves a relaxed version of the original problem,
	 * typically by relaxing integrality constraints, to obtain bounds
	 * on the optimal solution or to improve solution speed.
	 *
	 * @param timeLimit Maximum wall clock time for the solution process (default: CPX_INFBOUND).
	 * @return Status code indicating the result of the relaxation solution process.
	 */
	int solveRelaxation(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);

	/**
	 * @brief Change the type of a variable
	 *
	 * Modifies the type of the variable at the specified index. The original
	 * variable type is stored to allow restoration if needed.
	 *
	 * @param index Variable index
	 * @param type New variable type
	 * @return Reference to the current RlxFMIP object for method chaining
	 */
	RlxFMIP& changeVarType(const int index, const char type);

private:
	/**
	 * @brief Vector to store original variable types.
	 *
	 * This member variable maintains the original types of variables
	 * to allow restoration after relaxation operations.
	 */
	std::vector<char> restoreVarType;

	/**
	 * @brief Changes the problem type.
	 *
	 * This private method modifies the formulation of the problem by
	 * changing its type (e.g., from MILP to LP) according to the specified type code.
	 *
	 * @param type The new problem type code.
	 */
	void changeProbType(const int type);
};

#endif
/**
 * @file FMIP.hpp
 * @brief Specialized Mixed Integer Programming class that extends the base MIP class.
 *
 * This class provides functionality for Feasible Mixed Integer Programming problems,
 * with specialized algorithms and data structures optimized for feasibility checking.
 * It inherits core functionality from the base MIP class while adding FMIP-specific features.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.6
 * @since 06/23/2025
 */

#ifndef FMIP_H
#define FMIP_H

#include "MIP.hpp"

using namespace Utils;

/**
 * @class FMIP
 * @brief A derived class for Feasible Mixed Integer Programming problems.
 *
 * FMIP extends the base MIP class to provide specialized functionality for
 * handling feasible solutions to mixed integer programming problems.
 * This class contains methods for efficiently setting up and solving
 * feasibility-focused mixed integer programs.
 */
class FMIP : public MIP {

public:
	/**
	 * @brief Constructs an FMIP object from a file.
	 *
	 * @param fileName The path to the file containing the MIP problem data.
	 * @throws std::runtime_error If the file cannot be opened or contains invalid data.
	 */
	FMIP(std::string fileName);

	/**
	 * @brief Copy constructor.
	 *
	 * Creates a new FMIP object as a deep copy of another FMIP object.
	 *
	 * @param otherFMIP The FMIP object to copy.
	 */
	FMIP(const FMIP& otherFMIP);

	/**
	 * @brief Conversion constructor from MIP.
	 *
	 * Creates an FMIP object from a base MIP object, enabling conversion
	 * between the base class and this specialized class.
	 *
	 * @param otherMIP The MIP object to convert from.
	 */
	FMIP(const MIP& otherMIP);

	/**
	 * @brief Deleted assignment operator from MIP.
	 *
	 * Assignment from MIP to FMIP is not supported to prevent object slicing
	 * and maintain class invariants.
	 *
	 * @param other The MIP object to assign from.
	 * @return Reference to this FMIP object.
	 */
	FMIP& operator=(const MIP&) = delete;

	/**
	 * @brief Deleted assignment operator.
	 *
	 * Assignment between FMIP objects is not supported in the current implementation
	 * to prevent potential resource management issues.
	 *
	 * @param other The FMIP object to assign from.
	 * @return Reference to this FMIP object.
	 */
	FMIP& operator=(const FMIP&) = delete;

	/**
	 * @brief Gets the number of variables in the MIP.
	 *
	 * This method overrides the virtual function from the base MIP class.
	 *
	 * @return The number of variables in the Mixed Integer Program.
	 * @note This method does not throw exceptions (noexcept).
	 * @note The return value cannot be discarded (nodiscard attribute).
	 * @override Virtual method from MIP base class
	 */
	[[nodiscard]]
	size_t getMIPNumVars() noexcept override { return MIPNumVars; };

	[[nodiscard]]
	double getOMIPCost(const std::vector<double>& sol);

private:
	/**
	 * @brief Sets up the FMIP problem.
	 *
	 * This private method initializes internal data structures and
	 * prepares the problem for solving. It is called by the constructors.
	 */
	void setup();

	/**
	 * @brief The number of variables in the MIP.
	 *
	 * This member variable stores the total count of variables in the
	 * Mixed Integer Programming problem.
	 */
	size_t MIPNumVars;
	std::vector<double> ogObjFun;
};

#endif
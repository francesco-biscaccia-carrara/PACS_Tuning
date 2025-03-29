/**
 * @file OMIP.hpp
 * @brief Optimality Mixed Integer Programming class that extends the base MIP class.
 * 
 * This class provides functionality for Optimality Mixed Integer Programming problems,
 * with specialized algorithms and data structures for optimality. It contains methods
 * for handling budget constraints and analyzing slack variables in optimality problems.
 * It inherits core functionality from the base MIP class while adding optimality-specific features.
 * 
 * @author Francesco Biscaccia Carrara
 * @version v1.0.2
 * @since 03/29/2025
 */

#ifndef OMIP_H
#define OMIP_H

#include "MIP.hpp"

using namespace Utils;

/**
 * @class OMIP
 * @brief A derived class for Optimality Mixed Integer Programming problems.
 * 
 * OMIP extends the base MIP class to provide specialized functionality for
 * achieving optimality in mixed integer programming problems.
 * This class contains methods for efficiently handling budget constraints
 * and analyzing optimality parameters such as slack variables.
 */
class OMIP : public MIP {

public:

	/**
     * @brief Constructs an OMIP object from a file.
     * 
     * @param fileName The path to the file containing the MIP problem data.
     * @throws std::runtime_error If the file cannot be opened or contains invalid data.
     */
	OMIP(const std::string fileName);

	 /**
     * @brief Copy constructor.
     * 
     * Creates a new OMIP object as a deep copy of another OMIP object.
     * 
     * @param otherOMIP The OMIP object to copy.
     */
	OMIP(const OMIP& otherOMIP);

	/**
     * @brief Conversion constructor from MIP.
     * 
     * Creates an OMIP object from a base MIP object, enabling conversion
     * between the base class and this specialized optimization class.
     * 
     * @param otherMIP The MIP object to convert from.
     */
	OMIP(const MIP& otherMIP);

	/**
     * @brief Deleted assignment operator from MIP.
     * 
     * Assignment from MIP to OMIP is not supported to prevent object slicing
     * and maintain class invariants.
     * 
     * @param other The MIP object to assign from.
     * @return Reference to this OMIP object.
     */
	OMIP& operator=(const MIP&) = delete;

	/**
     * @brief Deleted assignment operator.
     * 
     * Assignment between OMIP objects is not supported in the current implementation
     * to prevent potential resource management issues.
     * 
     * @param other The OMIP object to assign from.
     * @return Reference to this OMIP object.
     */
	OMIP& operator=(const OMIP&) = delete;

	  /**
     * @brief Updates the budget constraint with a new right-hand side value.
     * 
     * This method modifies the existing budget constraint in the optimization problem
     * by setting a new right-hand side value, effectively changing the budget limit.
     * 
     * @param rhs The new right-hand side value (budget limit) for the constraint.
     * @return Reference to this OMIP object to allow method chaining.
     */
	OMIP& updateBudgetConstr(double rhs);

	 /**
     * @brief Gets the sum of all slack variables in the optimization problem.
     * 
     * Calculates and returns the total slack across all constraints in the model,
     * which can be used to analyze the tightness of the optimization solution.
     * 
     * @return The sum of all slack variables in the optimization problem.
     * @note The return value cannot be discarded (nodiscard attribute).
     */
	[[nodiscard]]
	double getSlackSum();
	
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
	int getMIPNumVars() noexcept override { return MIPNumVars; };

private:
/**
     * @brief Sets up the OMIP problem.
     * 
     * This private method initializes internal data structures and
     * prepares the optimization problem for solving. It is called by the constructors.
     */
	void setup();

	/**
     * @brief Adds a budget constraint to the optimization problem.
     * 
     * Creates and adds a new budget constraint with the specified right-hand side value.
     * This constraint typically limits the total resources available for the optimization.
     * 
     * @param rhs The right-hand side value (budget limit) for the constraint.
     */
	void addBudgetConstr(double rhs);

	 /**
     * @brief The number of variables in the MIP.
     * 
     * This member variable stores the total count of variables in the
     * Mixed Integer Programming optimization problem.
     */
	int	 MIPNumVars;
};

#endif
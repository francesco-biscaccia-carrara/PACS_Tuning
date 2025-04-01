/**
 * @file FixPolicy.hpp
 * @brief This file defines the FixPolicy namespace, which provides methods for handling 
 *        specific policy-based fixing strategies in mathematical optimization problems. 
 *        It includes exception handling and utility functions for modifying variables.
 * 
 * @author Francesco Biscaccia Carrara
 * @version v1.0.5
 * @since 03/29/2025
 */


#ifndef FIX_POL_H
#define FIX_POL_H

#include "RlxFMIP.hpp"

using namespace Utils;

namespace FixPolicy {

	/**
     * @class FixPolicyException
     * @brief Exception class for handling FixPolicy-related errors.
     */
	class FixPolicyException : public std::runtime_error {

	public:

		/**
         * @enum ExceptionType
         * @brief Enumerates different types of exceptions that can occur within FixPolicy.
         */
		enum class ExceptionType {
			General,
			InputSizeError,
			_count // Helper for array size
		};

		/**
         * @brief Constructs a FixPolicyException with a specific type and message.
         * @param type The type of exception.
         * @param message A descriptive error message.
         */
		explicit FixPolicyException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

	private:

		/**
         * @brief Mapping of exception types to their string representations.
         */
		static constexpr std::array<const char*, static_cast<size_t>(ExceptionType::_count)> typeNames = {
			"_general-ex_",
			"InputSizeError"
		};

		 /**
         * @brief Formats the exception message.
         * @param type The type of exception.
         * @param message The error message.
         * @return A formatted string containing the exception details.
         */
		static std::string formatMessage(ExceptionType type, const std::string& message) {
			return "FixPolicyException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
		}
	};

	/**
     * @brief Adjusts the theta values in the given vector.
     * @param x Vector of double values to be adjusted.
     * @param fileName Name of the file used for reference.
     * @param theta Parameter influencing the adjustment.
     * @param rnd Random number generator instance.
     */
	void firstThetaFixing(std::vector<double>& x,std::string fileName, double theta, Random rnd);

	/**
     * @brief Modifies the rho parameter in the given model based on a solution vector.
     * @param sol The solution vector.
     * @param model Reference to the MIP model being modified.
     * @param threadID ID of the thread executing this function.
     * @param rho Rho parameter value.
     * @param type Type of fixing applied.
     * @param rnd Random number generator instance.
     */
	void randomRhoFix(const std::vector<double>& sol, MIP& model, const size_t threadID, double rho,const char* type, Random& rnd);
};

#endif
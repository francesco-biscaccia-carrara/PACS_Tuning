/**
 * @file MergePolicy.hpp
 * @brief This file defines the MergePolicy namespace, which provides functions for handling 
 *        merging strategies in optimization models. It includes an exception handling class 
 *        and a function for recombining solutions.
 * 
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.4
 * @since 04/24/2025
 */

#ifndef MER_POL_H
#define MER_POL_H

#include "MIP.hpp"

using namespace Utils;

namespace MergePolicy {

	 /**
     * @class MergePolicyException
     * @brief Exception class for handling MergePolicy-related errors.
     */
	class MergePolicyException : public std::runtime_error {

	public:

		/**
         * @enum ExceptionType
         * @brief Enumerates different types of exceptions that can occur within MergePolicy.
         */
		enum class ExceptionType {
			General,
			InputSizeError,
			_count // Helper for array size
		};

		 /**
         * @brief Constructs a MergePolicyException with a specific type and message.
         * @param type The type of exception.
         * @param message A descriptive error message.
         */
		explicit MergePolicyException(ExceptionType type, const std::string& message) : std::runtime_error(formatMessage(type, message)){};

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
			return "MergePolicyException: [" + std::string(typeNames[static_cast<size_t>(type)]) + "] - " + std::string(message);
		}
	};

	/**
     * @brief Recombines multiple solutions into the given MIP model.
     * @param model Reference to the MIP model being modified.
     * @param x Vector of solutions to be merged.
     * @param phase The phase of the recombination process.
     */
	void recombine(MIP& model, const std::vector<Solution>& x, const char* phase);
};

#endif
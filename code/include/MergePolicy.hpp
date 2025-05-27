/**
 * @file MergePolicy.hpp
 * @brief This file defines the MergePolicy namespace, which provides functions for handling
 *        merging strategies in optimization models. It includes an exception handling class
 *        and a function for recombining solutions.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.2
 * @since 05/27/2025
 */

#ifndef MER_POL_H
#define MER_POL_H

#include "MIP.hpp"
#include "ACSException.hpp"

using namespace Utils;

namespace MergePolicy {

	/**
	 * @class MergePolicyException
	 * @brief Exception class for handling MergePolicy-related errors.
	 */
	class MergePolicyException : public ACSException{
		public :
			MergePolicyException(ExceptionType type,const std::string& message) : ACSException(type, message, "MergePolicy") {}
	};

	/**
	 * @brief Recombines multiple solutions into the given MIP model.
	 * @param model Reference to the MIP model being modified.
	 * @param x Vector of solutions to be merged.
	 * @param phase The phase of the recombination process.
	 */
	void recombine(MIP& model, const std::vector<Solution>& x, const char* phase);
}; // namespace MergePolicy

#endif
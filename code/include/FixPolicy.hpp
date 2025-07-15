/**
 * @file FixPolicy.hpp
 * @brief This file defines the FixPolicy namespace, which provides methods for handling
 *        specific policy-based fixing strategies in the ACS framework.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.2.11
 * @since 15/09/2025
 */

#ifndef FIX_POL_H
#define FIX_POL_H

#include <atomic>
#include <mutex>
#include <unordered_set>

#include "ACSException.hpp"
#include "RlxFMIP.hpp"
using namespace Utils;

#pragma region WALK_MIP_DEF

#define WALK_MIP_MIN_MOVE 32
#define WALK_MIP_HUGE_KICK 16
#define WALK_MIP_MAX_MOVE 1024
#define WALK_MIP_BETA (std::sqrt(5) - 1) / 2

#pragma endregion

#pragma region DYN_ADJUST_RHO_DEF
/** Clamping value for fixing */
#define MAX_UB 1e6

#pragma endregion

#pragma region DYN_ADJUST_RHO_DEF
/** Rho dicrepancy from the original one*/
#define DELTA_RHO 5e-2

/** Rho max value allowed */
#define MAX_RHO 99e-2
/** Rho min value allowed */
#define MIN_RHO 1e-2

#pragma endregion

namespace FixPolicy {

	/**
	 * @class FixPolicyException
	 * @brief Exception class for handling FixPolicy-related errors.
	 */
	class FixPolicyException : public ACSException {
	public:
		FixPolicyException(ExceptionType type, const std::string& message) : ACSException(type, message, "FixPolicy") {}
	};

	void startSolTheta(std::vector<double>& sol, std::string fileName, double theta, double timelimit, Random& rnd);

	/**
	 * @brief Modifies the sol vector to obtain a starting solution for FMIP optimization.
	 * @param sol Vector of double values to be updated.
	 * @param fileName Name of the file used to build the RelaxedFMIP object.
	 * @param rnd Random number generator instance.
	 */
	void startSolMaxFeas(std::vector<double>& sol, std::string fileName, Random& rnd);

	void walkMIPMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol, double rho, double p, Random& rnd);

	void fixSlackUpperBoundMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol);

	void fixSlackUpperBound(const char* type, MIP& model, const std::vector<double>& sol);

	/**
	 * @brief Modifies the rho parameter in the given model based on a solution vector.
	 * @param threadID ID of the thread executing this function.
	 * @param type Type of subMIP applied.
	 * @param model Reference to the MIP model being modified.
	 * @param sol The solution vector.
	 * @param rho Rho parameter value.
	 * @param rnd Random number generator instance.
	 */
	void randomRhoFixMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol, double rho, Random& rnd);

	/**
	 * @brief Adjusts Rho parameter dynamically to speed up ACS (in the recombination phases).
	 * @param phase String that define the phase.
	 * @param solveCode Code returned by CPXmipopt.
	 * @param numMIPs Number of logical sub-MIPs executing in parallel.
	 * @param CLIRho Reference rho value in the CLI args.
	 * @param A_RhoChanges Value of atomic size_t var used to handle adjustment.
	 */
	void dynamicAdjustRho(const char* phase, const int solveCode, const size_t numMIPs, double& CLIRho, const size_t A_RhoChanges);

	/**
	 * @brief Adjust Rho parameter to speed up ACS (multi-threading scenario).
	 * @param threadID ID of the thread executing this function.
	 * @param type String that define which type of subMIP is modifing rho.
	 * @param solveCode Code returned by CPXmipopt
	 * @param numMIPs Number of logical sub-MIPs executing in parallel.
	 * @param CLIRho Reference rho value in the CLI args.
	 * @param A_RhoChanges Reference to a atomic size_t var used to handle adjustment.
	 */
	void dynamicAdjustRhoMT(const size_t threadID, const char* type, const int solveCode, const size_t numMIPs, double& CLIRho, std::atomic_size_t& A_RhoChanges);
}; // namespace FixPolicy

#endif
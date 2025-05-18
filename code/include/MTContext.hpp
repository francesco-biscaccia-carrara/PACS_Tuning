/**
 * @file MT_CTX_H
 * @brief This file defines the MTContext class, which manages the context for multi-threaded optimization
 *        using FMIP (Feasibility MIP) and OMIP (Optimality MIP) methods.
 *
 * The MTContext class provides functionality for parallelizing optimization tasks across multiple threads,
 * maintaining temporary solutions, and ensuring thread synchronization when updating the best solution found
 * during the optimization process. It includes methods for starting optimization jobs, managing thread resources,
 * and updating shared solution data safely.
 *
 * It also handles the configuration of the number of threads and random number generators required for optimization,
 * and it provides mechanisms to broadcast solutions across threads and handle solution updates with thread safety.
 *
 * @author Francesco Biscaccia Carrara
 * @version v1.1.0 - InitSol v0.0.9
 * @since 05/18/2025
 */

#ifndef MT_CTX_H
#define MT_CTX_H

#include <atomic>
#include <mutex>
#include <thread>

#include "Utils.hpp"
using namespace Utils;

#include "FixPolicy.hpp"
using namespace FixPolicy;

#include "FMIP.hpp"
#include "OMIP.hpp"

/**
 * @class MTContext
 * @brief The MTContext class provides a context for managing multiple threads
 *        to optimize solutions using the FMIP and OMIP methods in parallel.
 *
 * This class is designed to handle optimization tasks with multiple threads,
 * sharing a common solution state and ensuring synchronization when updating
 * the best solution found during the optimization process.
 */
class MTContext {

public:
	/**
	 * @brief Constructs an MTContext object with a specified number of threads
	 *        and an initial random seed.
	 *
	 * @param subMIPNum The number of threads to be used for optimization (default is
	 *                  the number of hardware threads available).
	 * @param initialSeed The initial seed for random number generation (default is
	 *                    a random device seed).
	 */
	MTContext(size_t subMIPNum = std::thread::hardware_concurrency(), unsigned long long intialSeed = std::random_device{}());

	/**
	 * @brief Deleted copy constructor to prevent copying of MTContext objects.
	 */
	MTContext(const MTContext&) = delete;

	/**
	 * @brief Deleted assignment operator to prevent copying of MTContext objects.
	 */
	MTContext& operator=(const MTContext&) = delete;

	/**
	 * @brief Deleted move constructor to prevent moving of MTContext objects.
	 */
	MTContext(const MTContext&&) = delete;

	/**
	 * @brief Deleted move assignment operator to prevent moving of MTContext objects.
	 */
	MTContext& operator=(const MTContext&&) = delete;

	/**
	 * @brief Gets the best ACS incumbent solution.
	 *
	 * @return The best ACS incumbent solution.
	 */
	[[nodiscard]]
	inline const Solution& getBestACSIncumbent() { return bestACSIncumbent; }

	/**
	 * @brief Gets the number of times Rho has been changed.
	 * BE CAREFULL:: use only in a single-thread scenario!
	 *
	 * @return Number of times Rho has been changed.
	 */
	[[nodiscard]]
	inline size_t getRhoChanges() { return A_RhoChanges; }

	/**
	 * @brief Check if bestACSIncumbent is a feasible solution for the MIP problem
	 *
	 * @return Boolean statign wheter bestACSIncumbent is a feasible solution or not
	 */
	[[nodiscard]]
	inline bool isFeasibleSolFound() { return (bestACSIncumbent.slackSum <= EPSILON && bestACSIncumbent.oMIPCost < CPX_INFBOUND); }

	/**
	 * @brief Sets the best ACS incumbent solution.
	 *
	 * @param sol The solution to set as the best ACS incumbent.
	 */
	void setBestACSIncumbent(Solution& sol);

	// inline void setIncumbentAmongMIPsSize(size_t newSize) { incumbentAmongMIPs.sol.resize(newSize, 0.0); } FIXME: v0.0.9 - remove it

	/**
	 * @brief Resize the incumbent used in ACS.
	 *
	 * @param newSize The new size of the incumbent.
	 */
	inline void setBestACSIncumbentSize(size_t newSize) { bestACSIncumbent.sol.resize(newSize, 0.0); }

	/**
	 * @brief Gets the temporary solutions stored for optimization.
	 *
	 * @return A reference to the vector of temporary solutions.
	 */
	[[nodiscard]]
	inline const std::vector<Solution>& getTmpSolutions() { return tmpSolutions; }

	/**
	 * @brief Broadcasts a temporary solution to all threads.
	 *
	 * @param tmpSol The temporary solution to broadcast.
	 * @return Reference to the current MTContext object.
	 */
	MTContext& broadcastSol(Solution& tmpSol);

	/**
	 * @brief Gets the number of threads used for optimization.
	 *
	 * @return The number of threads.
	 */
	[[nodiscard]]
	inline size_t getNumThreads() { return numMIPs; }

	/**
	 * @brief Starts parallel optimization using the FMIP method.
	 *
	 * @param CLIArgs The command-line arguments for optimization.
	 * @return Reference to the current MTContext object.
	 */
	MTContext& parallelFMIPOptimization(Args& CLIArgs);

	/**
	 * @brief Starts parallel optimization using the OMIP method.
	 * @param slackSumUB The slack upper bound used in the OMIP method.
	 * @param CLIArgs The command-line arguments for optimization.
	 * @return Reference to the current MTContext object.
	 */
	MTContext& parallelOMIPOptimization(const double slackSumUB, Args& CLIArgs);

	// MTContext& parallelInitSolMerge(std::string fileName, std::vector<double>& sol, Random& rnd); FIXME: v0.0.9 - remove it

	/**
	 * @brief Destructor for MTContext. Cleans up resources used by the context.
	 */
	~MTContext();

private:
	size_t					 numMIPs;		   ///< Number of threads used for optimization.
	std::vector<Solution>	 tmpSolutions;	   ///< Temporary solutions for optimization.
	std::vector<std::thread> threads;		   ///< Threads used for parallel optimization.
	std::vector<Random>		 rndGens;		   ///< Random number generators for each thread.
	Solution				 bestACSIncumbent; ///< Best ACS incumbent solution found.
	// Solution				 incumbentAmongMIPs; FIXME: v0.0.9 - remove it
	std::mutex				 MTContextMTX; ///< Mutex for synchronizing solution updates.
	std::atomic_size_t		 A_RhoChanges; ///< Size_t value used to manage the DynamicFixPolicy

	/**
	 * @brief Waits for all threads to complete their optimization jobs.
	 */
	void waitAllJobs();

	/**
	 * @brief Sets a temporary solution at the specified index.
	 *
	 * @param index The index of the solution to set.
	 * @param tmpSol The temporary solution to set.
	 */
	inline void setTmpSolution(int index, Solution& tmpSol) { tmpSolutions[index] = tmpSol; }

	/**
	 * @brief Runs the FMIP optimization job for a given thread.
	 *
	 * @param thID The ID of the thread running the job.
	 * @param CLIArgs The command-line arguments for the optimization process.
	 */
	void FMIPInstanceJob(const size_t thID, Args& CLIArgs);

	/**
	 * @brief Runs the OMIP optimization job for a given thread.
	 *
	 * @param thID The ID of the thread running the job.
	 * @param CLIArgs The command-line arguments for the optimization process.
	 * @param slackSumUB The slack upper bound used in the OMIP method.
	 */
	void OMIPInstanceJob(const size_t thID, const double slackSumUB, Args& CLIArgs);

	
	// void initSolMergeJob(const size_t thID, std::string fileName); FIXME: v0.0.9 - remove it
};

#endif
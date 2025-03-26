#ifndef MT_CTX_H
#define MT_CTX_H

#include <mutex>
#include <thread>

#include "Utils.hpp"
using namespace Utils;

#include "FixPolicy.hpp"
using namespace FixPolicy;

#include "FMIP.hpp"
#include "OMIP.hpp"


class MTContext {

public:
	MTContext(size_t subMIPNum = std::thread::hardware_concurrency(), unsigned long long intialSeed = std::random_device{}());
	MTContext(const MTContext&) = delete;
	MTContext& operator=(const MTContext&) = delete;
	MTContext(const MTContext&&) = delete;
	MTContext& operator=(const MTContext&&) = delete;

	[[nodiscard]]
	inline Solution getBestACSIncumbent() { return bestACSIncumbent; }
	MTContext&		setBestACSIncumbent(Solution& sol);

	[[nodiscard]]
	inline const std::vector<Solution>& getTmpSolutions() { return tmpSolutions; }
	MTContext&		broadcastSol(Solution& tmpSol);

	[[nodiscard]]
	inline size_t getNumThreads() { return numMIPs; }

	MTContext& parallelFMIPOptimization(double remTime, Args CLIArgs);
	MTContext& parallelOMIPOptimization(double remTime, Args CLIArgs, double slackSumUB);

	~MTContext();

private:
	size_t					 numMIPs;
	std::vector<Solution>	 tmpSolutions;
	std::vector<std::thread> threads;
	std::vector<Random>		 rndGens;
	Solution				bestACSIncumbent;
	std::mutex				 updateSolMTX;

	void waitAllJobs();

	inline void		setTmpSolution(int index, Solution& tmpSol){tmpSolutions[index] = tmpSol;}
	[[nodiscard]]
	inline Solution getTmpSolution(int index) { return tmpSolutions[index]; }
	void FMIPInstanceJob(size_t thID, double remTime, Args CLIArgs);
	void OMIPInstanceJob(size_t thID, double remTime, Args CLIArgs, double slackSumUB);
};

#endif
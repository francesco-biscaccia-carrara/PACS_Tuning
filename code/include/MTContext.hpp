#ifndef MT_CTX_H
#define MT_CTX_H

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
    inline Solution getBestIncumbent(){return bestIncumbent;}
    MTContext& setBestIncumbent(Solution sol);

    MTContext& setTmpSolution(int index, Solution tmpSol);

    [[nodiscard]]
    inline size_t getNumThreads(){return threadIDs.size();}
    
    MTContext& parallelFMIPOptimization(double remTime, Args CLIArgs);
    MTContext& parallelOMIPOptimization(double remTime, double detTL);

    MTContext& waitAllJobs();
    ~MTContext();

private:
    std::vector<Solution> tmpSolutions;
    std::vector<std::thread> threads;
    std::vector<size_t> threadIDs; 
    std::vector<Random> rndGens;
    Solution bestIncumbent;
    std::mutex updateSolMTX; 
};

#endif
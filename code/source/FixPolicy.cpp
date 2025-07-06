#include "../include/FixPolicy.hpp"

using FPEx = FixPolicy::FixPolicyException::ExceptionType;
using MIPEx = MIPException::ExceptionType;

bool isInteger(double n) {
	return ((n < CPX_INFBOUND) && (static_cast<int>(n) == n));
}

void FixPolicy::startSolTheta(std::vector<double>& sol, std::string fileName, double theta, double timelimit, Random& rnd) {
	if (theta < EPSILON || theta > 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Theta par. must be within (0,1)!");

	RlxFMIP relaxedFMIP{ fileName };
	size_t		numVarsToFix{relaxedFMIP.getMIPNumVars()};
	sol.resize(numVarsToFix, CPX_INFBOUND);

	std::vector<size_t> varRangesIndices(numVarsToFix);
	std::iota(varRangesIndices.begin(), varRangesIndices.end(), 0);
	std::sort(varRangesIndices.begin(), varRangesIndices.end(), [&relaxedFMIP](const int& a, const int& b) {
		auto [aLowerBound, aUpperBound] = relaxedFMIP.getVarBounds(a);
		auto [bLowerBound, bUpperBound] = relaxedFMIP.getVarBounds(b);
		return (aUpperBound - aLowerBound) < (bUpperBound - bLowerBound);
	});

	std::vector<bool> isFixed(numVarsToFix, false);
	size_t			  numFixedVars = 0;

	while (numFixedVars < numVarsToFix) {

		size_t numNotFixedVars{ numVarsToFix - numFixedVars };
		size_t varsToFix{ static_cast<size_t>(std::ceil(numNotFixedVars * theta)) };

		size_t fixedThisIteration{ 0 };

		for (size_t i = 0; i < numVarsToFix && fixedThisIteration < varsToFix; i++) {
			int idx = varRangesIndices[i];
			if (!isFixed[idx]) {
				auto [lowerBound, upperBound] = relaxedFMIP.getVarBounds(idx);

				double clampedLower = std::max(-MAX_UB, lowerBound);
				double clampedUpper = std::min(MAX_UB, upperBound);

				sol[idx] = rnd.Int(clampedLower, clampedUpper);
				relaxedFMIP.setVarValue(idx, sol[idx]);
				isFixed[idx] = true;
				fixedThisIteration++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("FixPolicy::startSolTheta - %zu vars hard-fixed", varsToFix);
#endif

		relaxedFMIP.solveRelaxation(Clock::timeRemaining(timelimit),DET_TL(relaxedFMIP.getNumNonZeros()));

		std::vector<double> lpSol = relaxedFMIP.getSol();
		for (size_t i = 0; i < numVarsToFix; ++i) {
			if (isFixed[i])
				continue;
			if (isInteger(lpSol[i])) {
				sol[i] = lpSol[i];
				relaxedFMIP.setVarValue(i, sol[i]);
				isFixed[i] = true;
			}
		}

		numFixedVars = std::accumulate(isFixed.begin(), isFixed.end(), 0);
	}
}

void FixPolicy::startSolMaxFeas(std::vector<double>& sol, std::string fileName, Random& rnd) {

	MIP	   MIP{ fileName };
	size_t numVarsToFix{ MIP.getMIPNumVars()};
	sol.resize(numVarsToFix, CPX_INFBOUND);

	std::vector<VarBounds> varRanges(numVarsToFix);
	for (size_t i{ 0 }; i < numVarsToFix; i++)
		varRanges[i] = MIP.getVarBounds(i);

	std::vector<double> obj = MIP.getObjFunction();

#if ACS_VERBOSE >= VERBOSE
	size_t zeros = 0, lbs = 0, ubs = 0, rnds = 0;
#endif
	for (size_t i{ 0 }; i < numVarsToFix; i++) {
		auto [lb, ub] = varRanges[i];

		if (lb == -CPX_INFBOUND && ub == CPX_INFBOUND) {
			sol[i] = 0;
#if ACS_VERBOSE >= VERBOSE
			zeros++;
#endif
		} else if (lb >= -CPX_INFBOUND && ub == CPX_INFBOUND) {
			sol[i] = lb;
#if ACS_VERBOSE >= VERBOSE
			lbs++;
#endif
		} else if (lb == -CPX_INFBOUND && ub <= CPX_INFBOUND) {
			sol[i] = ub;
#if ACS_VERBOSE >= VERBOSE
			ubs++;
#endif
		} else {
			if (obj[i] <= -EPSILON) {
				sol[i] = lb;
#if ACS_VERBOSE >= VERBOSE
				lbs++;
#endif
			} else if (obj[i] >= EPSILON) {
				sol[i] = ub;
#if ACS_VERBOSE >= VERBOSE
				ubs++;
#endif
			} else {
				sol[i] = rnd.Int(lb, ub);
#if ACS_VERBOSE >= VERBOSE
				rnds++;
#endif
			}
		}
	}
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("FixPolicy::startSolMaxFeas - %zu zeros vars | %zu vars at LB | %zu vars at UB | %zu random vars", zeros, lbs, ubs, rnds);
#endif
}

static std::once_flag initFlag;
static int initViolConst = -1;

void FixPolicy::walkMIPMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol, double p, Random& rnd) {
	if (p < EPSILON || p >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "WalkProb par. must be within (0,1)!");

	std::vector<double> tmpSol(sol.begin(), sol.begin() + model.getMIPNumVars());
	double 				initViol = model.violation(tmpSol);

	std::vector<VarBounds> ogVarBounds;
	for (size_t i{ 0 }; i < tmpSol.size();i++){
		ogVarBounds.push_back(model.getVarBounds(i));
	}

	std::vector<int> violConstr = model.getViolatedConstrIndex(tmpSol);
	if(violConstr.empty()) return;
	
	std::call_once(initFlag, [&]() {initViolConst = static_cast<int>(violConstr.size());});

#if ACS_VERBOSE >= VERBOSE
	size_t				bestMoves = 0, minDMGMoves =0 , rndMoves = 0;
#endif

	double ratio = static_cast<double>(violConstr.size()) / std::max(static_cast<double>(initViolConst), 1.0);
	ratio = std::clamp(ratio, 0.0, 1.0);

	size_t numVarToFix = static_cast<size_t>(std::ceil(WALK_MIP_MIN_MOVE + (WALK_MIP_MAX_MOVE - WALK_MIP_MIN_MOVE) * std::pow(ratio, WALK_MIP_BETA)));
	size_t		ogNumRows = model.getMIPrmatbeg().size();

	const auto& rmatbeg = model.getMIPrmatbeg();
	const auto& rmatind = model.getMIPrmatind();

	std::vector<std::vector<int>> varToConstr(model.getMIPNumVars());   // Variable → constraints

	//Saving part of the bipartite graph var-cons involved 
	for (size_t c: violConstr) {
    	int start = rmatbeg[c];
    	int end   = (c == ogNumRows - 1) ? rmatind.size() : rmatbeg[c + 1];
    	for (int i = start; i < end; ++i) {
        	int var = rmatind[i];
        	varToConstr[var].push_back(c);
    	}
	}

	while (numVarToFix-- > 0) {
		size_t	rndConstrInd = violConstr[rnd.Int(0, violConstr.size() - 1)];

		int 	start = rmatbeg[rndConstrInd];
		int	   	end = (rndConstrInd == ogNumRows - 1) ? rmatind.size() : rmatbeg[rndConstrInd + 1];

		double				minDMG = std::numeric_limits<double>::max();
		int 				candVar = -1;
		double				perturb = 0.0;
		double				newVal = 0.0;

		for (int j = start; j < end; j++){ 
			int varIndex = rmatind[j];

			switch (model.getVarType(varIndex)) {
				case CPX_BINARY :{
					int tmpBin = not tmpSol[varIndex];
					perturb = tmpBin - tmpSol[varIndex];
					break;
				}

				case CPX_INTEGER :{
					int tmpInt = tmpSol[varIndex] + ((rnd.Double(0, 1) <= 0.5) ? -1 : 1);

					auto [lb, ub] = ogVarBounds[varIndex];
					if(tmpInt>= lb && tmpInt<=ub) perturb = tmpInt - tmpSol[varIndex];
					else continue;
					break;
				}
					

				case CPX_CONTINUOUS :{
					double tmpDouble = tmpSol[varIndex] + rnd.Double(-0.5,0.5);

					auto [lb, ub] = ogVarBounds[varIndex];
					if(tmpDouble>= lb && tmpDouble<=ub) perturb = tmpDouble - tmpSol[varIndex];
					else continue;
					break;
				}

				default:
					throw MIPException(MIPEx::GetFunction, "Unknown variable type");
				break;
			}

			double delta = model.violationVarDelta(varIndex,perturb,varToConstr[varIndex]);
			if (delta < minDMG) {
				minDMG = delta;
				candVar = varIndex;
				newVal = tmpSol[varIndex]+perturb;
			}
		}

		if (minDMG <= -EPSILON) {
			tmpSol[candVar] = newVal;
			model.setVarValue(candVar, tmpSol[candVar]);
#if ACS_VERBOSE >= VERBOSE
			bestMoves++;
#endif
		} else {
			if(rnd.Double(0,1) <= p && candVar!=-1){
				tmpSol[candVar] = newVal;
				model.setVarValue(candVar, tmpSol[candVar]);
#if ACS_VERBOSE >= VERBOSE
				minDMGMoves++;
#endif
			} else {
					int rndVar = rmatind[rnd.Int(start,end-1)];

					switch (model.getVarType(rndVar)) {
						case CPX_BINARY: {
							model.setVarValue(rndVar, not tmpSol[rndVar]);
							break;
						}

						case CPX_INTEGER: {
							auto [lb, ub] = ogVarBounds[rndVar];

							tmpSol[rndVar] = rnd.Int(lb, ub);
							model.setVarValue(rndVar, tmpSol[rndVar]);
							break;
						}

						case CPX_CONTINUOUS: {
							auto [lb, ub] = ogVarBounds[rndVar];
					
							tmpSol[rndVar] = rnd.Double(lb, ub);;
							model.setVarValue(rndVar, tmpSol[rndVar]);
							break;
						}

						default: {
							throw MIPException(MIPEx::GetFunction, "Unknown variable type");
							break;
						}
					}
#if ACS_VERBOSE >= VERBOSE
				rndMoves++;
#endif
			}
		}
	}
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::walkMIPMT - Viol before|after: %10.2f|%-10.2f \n\t\t\
		    - FixPolicy::walkMIPMT - Best Moves: %3zu| Min-Damage Moves: %3zu| RND Moves: %3zu", threadID, type, initViol , model.violation(tmpSol) ,bestMoves, minDMGMoves, rndMoves);
#endif
}

void FixPolicy::fixSlackUpperBoundMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol) {

		size_t numMIPVars{ model.getMIPNumVars() };
		size_t numVars{ model.getNumCols() };

		if (sol.size() != numVars)
			throw FixPolicyException(FPEx::InputSizeError, "Incosistent length: sol.size = " + std::to_string(sol.size()) + ", numVars = " + std::to_string(numVars));

		size_t fixedToUBVars = 0;
		for (size_t i{ numMIPVars }; i < numVars; i++) {

			auto [lb, ub] = model.getVarBounds(i);
			if (ub - sol[i] > EPSILON) {
				model.setVarUpperBound(i, sol[i]);
				fixedToUBVars++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::fixSlackUpperBoundMT - %4d vars UB updated",threadID,type,fixedToUBVars);
#endif

}

void FixPolicy::fixSlackUpperBound(const char* phase, MIP& model, const std::vector<double>& sol) {

	size_t numMIPVars{ model.getMIPNumVars()};
	size_t numVars{ model.getNumCols()};

	if(sol.size() != numVars)
		throw FixPolicyException(FPEx::InputSizeError, "Incosistent length: sol.size = "+std::to_string(sol.size())+", numVars = "+std::to_string(numVars));

	size_t fixedToUBVars = 0;
	for (size_t i{ numMIPVars }; i < numVars; i++) {

		auto [lb, ub] = model.getVarBounds(i);
		if ( ub - sol[i] > EPSILON){
			model.setVarUpperBound(i, sol[i]);
			fixedToUBVars++;
		}
	}

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("[%s] - FixPolicy::fixSlackUpperBound - Fixed UB of %10d vars",phase,fixedToUBVars);
#endif
}

void FixPolicy::randomRhoFixMT(const size_t threadID, const char* type, MIP& model, const std::vector<double>& sol, double rho, Random& rnd) {
	if (rho < EPSILON || rho >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Rho par. must be within (0,1)!");

	size_t		 xLen{ model.getMIPNumVars()};
	const size_t numFixedVars = static_cast<size_t>(rho * xLen);
	const size_t start = rnd.Int(0, xLen - 1);

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::randomRhoFixMT - %zu vars hard-fixed [%5.4f]", threadID, type, numFixedVars, rho);
#endif

	for (size_t i{ 0 }; i < numFixedVars; i++) {
		size_t index{ (start + i) % xLen };
		model.setVarValue(index, sol[index]);
	}
}

void FixPolicy::dynamicAdjustRho(const char* phase, const int solveCode, const size_t numMIPs, double& CLIRho, const size_t A_RhoChanges) {
	if (A_RhoChanges >= numMIPs)
		return;

	double scaledDeltaRho = 2 * DELTA_RHO / numMIPs;

	switch (solveCode) {
		case CPXMIP_OPTIMAL:
		case CPXMIP_OPTIMAL_TOL:
			CLIRho = (CLIRho - scaledDeltaRho < MIN_RHO) ? MIN_RHO : CLIRho - scaledDeltaRho;
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("[%s] - FixPolicy::dynamicAdjustRho - Rho Decreased [%5.4f]", phase, CLIRho);
#endif
			break;

		case CPXMIP_DETTIME_LIM_FEAS:
		case CPXMIP_TIME_LIM_FEAS:
			CLIRho = (CLIRho + scaledDeltaRho > MAX_RHO) ? MAX_RHO : CLIRho + scaledDeltaRho;
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("[%s] - FixPolicy::dynamicAdjustRho - Rho Increased [%5.4f]", phase, CLIRho);
#endif
			break;

		default:
#if ACS_VERBOSE >= VERBOSE
			PRINT_ERR("Unexpected value for solvecode: %d", solveCode);
#endif

			break;
	}
}

static std::atomic<size_t> numDecRho{0};
static std::atomic<size_t> numIncRho{0};
static std::mutex rhoMutex;
static std::once_flag resetFlag;

void FixPolicy::dynamicAdjustRhoMT(const size_t threadID, const char* type, const int solveCode, const size_t numMIPs, double& CLIRho, std::atomic_size_t& A_RhoChanges) {
    
    if (A_RhoChanges.load() >= numMIPs)
        return;
    
    // Reset counters once when first thread reaches here
    if (A_RhoChanges.load() == 0) {
        std::call_once(resetFlag, []() {
            numDecRho.store(0);
            numIncRho.store(0);
        });
    }
    
    double scaledDeltaRho = DELTA_RHO / numMIPs;
    
    // Use mutex only for CLIRho modifications
    switch (solveCode) {
        case CPXMIP_OPTIMAL:
        case CPXMIP_OPTIMAL_TOL: {
            std::lock_guard<std::mutex> lock(rhoMutex);
            if (A_RhoChanges.load() >= numMIPs) return; // Double-check
            
            CLIRho = (CLIRho - scaledDeltaRho < MIN_RHO) ? MIN_RHO : CLIRho - scaledDeltaRho;
            A_RhoChanges.fetch_add(1);
            numDecRho.fetch_add(1);
#if ACS_VERBOSE >= VERBOSE
            PRINT_INFO("Proc: %3d [%s] - FixPolicy::dynamicAdjustRhoMT - Rho Decreased [%5.4f]", 
                      threadID, type, CLIRho);
#endif
            break;
        }
        
        case CPXMIP_DETTIME_LIM_FEAS:
        case CPXMIP_TIME_LIM_FEAS: {
            std::lock_guard<std::mutex> lock(rhoMutex);
            if (A_RhoChanges.load() >= numMIPs) return; // Double-check
            
            CLIRho = (CLIRho + scaledDeltaRho > MAX_RHO) ? MAX_RHO : CLIRho + scaledDeltaRho;
            A_RhoChanges.fetch_add(1);
            numIncRho.fetch_add(1);
#if ACS_VERBOSE >= VERBOSE
            PRINT_INFO("Proc: %3d [%s] - FixPolicy::dynamicAdjustRhoMT - Rho Increased [%5.4f]", 
                      threadID, type, CLIRho);
#endif
            break;
        }
        
        default:
#if ACS_VERBOSE >= VERBOSE
            PRINT_ERR("Unexpected value for solvecode: %d", solveCode);
#endif
            break;
    }
    
    // Coinflip case handling (also needs mutex for CLIRho access)
    {
        std::lock_guard<std::mutex> lock(rhoMutex);
        size_t currentChanges = A_RhoChanges.load();
        if (currentChanges == numMIPs && numIncRho.load() == numDecRho.load()) {
            CLIRho = (CLIRho - DELTA_RHO < MIN_RHO) ? MIN_RHO : CLIRho - DELTA_RHO;
#if ACS_VERBOSE >= VERBOSE
            PRINT_WARN("[\"COINFLIP\" CASE] - FixPolicy::dynamicAdjustRhoMT - Rho decreased [%5.4f]", CLIRho);
#endif
        }
    }
}
#include "../include/FixPolicy.hpp"

using FPEx = FixPolicy::FixPolicyException::ExceptionType;

bool isInteger(double n) {
	return ((n < CPX_INFBOUND) && (static_cast<int>(n) == n));
}

void FixPolicy::startSolTheta(std::vector<double>& sol, std::string fileName, double theta, Random& rnd) {
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

		relaxedFMIP.solveRelaxation();

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

// Values to count the number of decrements/increments of Rho parameter in 1 iteration
static size_t numDecRho = 0;
static size_t numIncRho = 0;

void FixPolicy::dynamicAdjustRhoMT(const size_t threadID, const char* type, const int solveCode, const size_t numMIPs, double& CLIRho, std::atomic_size_t& A_RhoChanges) {

	if (A_RhoChanges >= numMIPs)
		return; // Whenever an incumbent is found, we don't touch Rho anymore
	if (A_RhoChanges == 0)
		numDecRho = numIncRho = 0;

	double scaledDeltaRho = DELTA_RHO / numMIPs;

	switch (solveCode) {
		case CPXMIP_OPTIMAL:
		case CPXMIP_OPTIMAL_TOL:
			CLIRho = (CLIRho - scaledDeltaRho < MIN_RHO) ? MIN_RHO : CLIRho - scaledDeltaRho;
			A_RhoChanges++;
			numDecRho++;
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("Proc: %3d [%s] - FixPolicy::dynamicAdjustRhoMT - Rho Decreased [%5.4f]", threadID, type, CLIRho);
#endif
			break;

		case CPXMIP_DETTIME_LIM_FEAS:
		case CPXMIP_TIME_LIM_FEAS:
			CLIRho = (CLIRho + scaledDeltaRho > MAX_RHO) ? MAX_RHO : CLIRho + scaledDeltaRho;
			A_RhoChanges++;
			numIncRho++;
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("Proc: %3d [%s] - FixPolicy::dynamicAdjustRhoMT - Rho Increased [%5.4f]", threadID, type, CLIRho);
#endif
			break;

		default:
#if ACS_VERBOSE >= VERBOSE
			PRINT_ERR("Unexpected value for solvecode: %d", solveCode);
#endif

			break;
	}

	// Policy in the coin-flip case: make the problem harder
	if (A_RhoChanges == numMIPs && numIncRho == numDecRho) {
		CLIRho = (CLIRho - DELTA_RHO < MIN_RHO) ? MIN_RHO : CLIRho - DELTA_RHO;
#if ACS_VERBOSE >= VERBOSE
		PRINT_WARN("[\"COINFLIP\" CASE] - FixPolicy::dynamicAdjustRhoMT - Rho decreased [%5.4f]", CLIRho);
#endif
	}
}
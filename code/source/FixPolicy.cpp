#include "../include/FixPolicy.hpp"

#define MAX_UB 1e6

using FPEx = FixPolicy::FixPolicyException::ExceptionType;

bool isInteger(double n) {
	return ((n < CPX_INFBOUND) && (static_cast<int>(n) == n));
}

void FixPolicy::firstThetaFixing(std::vector<double>& x,std::string fileName, double theta, Random rnd) {
	if (theta < EPSILON || theta > 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Theta par. must be within (0,1)!");


	RlxFMIP	 relaxedFMIP{ fileName };
	int		 numVarsToFix{ relaxedFMIP.getMIPNumVars() };
	x.resize(numVarsToFix,CPX_INFBOUND);	

	
	std::vector<size_t>					varRangesIndices(numVarsToFix);
	std::iota(varRangesIndices.begin(), varRangesIndices.end(), 0);
	std::sort(varRangesIndices.begin(),varRangesIndices.end(), [&relaxedFMIP](const int& a,const int& b){
        auto [aLowerBound, aUpperBound]  = relaxedFMIP.getVarBounds(a);
        auto [bLowerBound, bUpperBound] = relaxedFMIP.getVarBounds(b);
        return (aUpperBound - aLowerBound) <  (bUpperBound - bLowerBound); 
    });


	std::vector<bool> isFixed(numVarsToFix, false);
	size_t			  numFixedVars = 0;

	while (numFixedVars < numVarsToFix) {
		 
		size_t numNotFixedVars {numVarsToFix - numFixedVars};
		size_t varsToFix {static_cast<size_t>(std::ceil(numNotFixedVars * theta))};

		size_t fixedThisIteration {0};

		for (size_t i = 0; i < numVarsToFix && fixedThisIteration < varsToFix; i++) {
			int idx = varRangesIndices[i];
			if (!isFixed[idx]) {
				auto [lowerBound, upperBound] = relaxedFMIP.getVarBounds(idx);

				double clampedLower = std::max(-MAX_UB, lowerBound);
				double clampedUpper = std::min(MAX_UB, upperBound);

				x[idx] = rnd.Int(clampedLower, clampedUpper);
				relaxedFMIP.setVarValue(idx,x[idx]);
				isFixed[idx] = true;
				fixedThisIteration++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("FixPolicy::firstThetaFixing - %zu vars hard-fixed", varsToFix);
#endif

		relaxedFMIP.solveRelaxation();

		std::vector<double> lpSol = relaxedFMIP.getSol();
		for (size_t i = 0; i < numVarsToFix; ++i) {
			if(isFixed[i]) continue;
			if(isInteger(lpSol[i])) {
				x[i] = lpSol[i];
				relaxedFMIP.setVarValue(i,x[i]);
				isFixed[i] = true;
			}
		}

		numFixedVars = std::accumulate(isFixed.begin(),isFixed.end(),0);
	}

}

void FixPolicy::randomRhoFix(const std::vector<double>& sol, MIP& model, const size_t threadID, double rho,const char* type, Random& rnd) {
	if (rho < EPSILON || rho >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Rho par. must be within (0,1)!");

	size_t xLen{static_cast<size_t>(model.getMIPNumVars())};
	const size_t numFixedVars = static_cast<size_t>(rho *xLen);
	const size_t start = rnd.Int(0,xLen - 1);

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::randomRhoFix - %zu vars hard-fixed", threadID, type, numFixedVars);
#endif

	for (size_t i{ 0 }; i < numFixedVars; i++) {
		size_t index{ (start + i) % xLen};
		model.setVarValue(index, sol[index]);
	}
}


void FixPolicy::dynamicAdjustRho(const int solveCode, Args& CLI){
	// if solution is better then the one stored in ACSIncumbent (both on fMIP sol and oMIP sol) --> dont't change rho
	// #include <atomic> , std::atomic_bool ArhoAdjusted; to prevent multiple adjustment of rho
	// else adjust 
	switch(solveCode){
		case CPXMIP_OPTIMAL:
		case CPXMIP_OPTIMAL_TOL:
			CLI.rho-=DELTA_RHO;
#if ACS_VERBOSE >= VERBOSE
			PRINT_INFO("CPXMIP_OPTIMAL|CPXMIP_OPTIMAL_TOL --> Rho reduced");
#endif
			break;
	}
}
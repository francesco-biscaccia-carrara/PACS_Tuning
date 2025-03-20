#include "../include/FixPolicy.hpp"

#define MAX_UB 1e6

using FPEx = FixPolicy::FixPolicyException::ExceptionType;

bool isInteger(double n) {
	return ((std::floor(n) == n) && (n != CPX_INFBOUND));
}

Solution FixPolicy::firstThetaFixing(std::string fileName, double theta, Random rnd) {
	if (theta < EPSILON || theta >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Theta par. must be within (0,1)!");

	RlxMIP	 relaxedFMIP{ fileName };
	int		 numVarsToFix{ relaxedFMIP.getMIPNumVars() };

	Solution rtn = { .sol = std::vector(relaxedFMIP.getNumCols(), CPX_INFBOUND),
					 .slackSum = CPX_INFBOUND,
					 .oMIPCost = CPX_INFBOUND };

	
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
		size_t varsToFix {static_cast<size_t>(numNotFixedVars * theta)};

		size_t fixedThisIteration {0};

		for (size_t i = 0; i < numVarsToFix && fixedThisIteration < varsToFix; i++) {
			int idx = varRangesIndices[i];
			if (!isFixed[idx]) {
				auto [lowerBound, upperBound] = relaxedFMIP.getVarBounds(idx);

				double clampedLower = std::max(-MAX_UB, lowerBound);
				double clampedUpper = std::min(MAX_UB, upperBound);

				rtn.sol[idx] = rnd.Int(clampedLower, clampedUpper);
				relaxedFMIP.setVarValue(idx,rtn.sol[idx]);
				isFixed[idx] = true;
				numFixedVars++;
				fixedThisIteration++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("FixPolicy::firstThetaFixing - %zu vars hard-fixed", varsToFix);
#endif

		relaxedFMIP.solveRelaxation();

		std::vector<double> lpSol = relaxedFMIP.getSol();
		for (size_t i = 0; i < lpSol.size(); ++i) {
			if (isInteger(lpSol[i])) {
				rtn.sol[i] = lpSol[i];
				relaxedFMIP.setVarValue(i,rtn.sol[i]);
				isFixed[i] = true;
				numFixedVars++;
			}
		}
	}

	for (size_t i{ 0 }; i < rtn.sol.size();i++){
		relaxedFMIP.setVarValue(i, rtn.sol[i]);
	}
	relaxedFMIP.solve();
	rtn.sol.resize(numVarsToFix); //TODO: Kinda sus
	rtn.slackSum = relaxedFMIP.getObjValue();
	return rtn;
}

void FixPolicy::randomRhoFix(const std::vector<double>& sol, MIP& model, const size_t threadID, double rho, const char* type, Random& rnd) {
	if (rho < EPSILON || rho >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Rho par. must be within (0,1)!");

	const size_t numFixedVars = static_cast<size_t>(rho * sol.size());
	const size_t start = rnd.Int(0, sol.size() - 1);

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::randomRhoFix - %zu vars hard-fixed", threadID, type, numFixedVars);
#endif

	for (size_t i{ 0 }; i < numFixedVars; i++) {
		size_t index{ (start + i) % sol.size() };
		model.setVarValue(index, sol[index]);
	}
}
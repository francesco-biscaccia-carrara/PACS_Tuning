#include "../include/FixPolicy.hpp"

#define MAX_UB 1e6

using FPEx = FixPolicy::FixPolicyException::ExceptionType;

bool isInteger(double n) {
	return ((std::floor(n) == n) && (n != CPX_INFBOUND));
}

bool allInteger(std::vector<double>& x) {
	for (auto e : x)
		if (!isInteger(e))
			return false;
	return true;
}

Solution FixPolicy::firstThetaFixing(std::string fileName, double theta, Random rnd) {
	if (theta < EPSILON || theta >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Theta par. must be within (0,1)!");

	RlxMIP relaxedFMIP{fileName};
	int numVars{relaxedFMIP.getMIPNumVars()};
	Solution rtn ={.sol = std::vector(numVars,CPX_INFBOUND),.slackSum = CPX_INFBOUND, .oMIPCost= CPX_INFBOUND};

	std::vector<std::pair<int, double>> varRanges(numVars);
	for (size_t i = 0; i < numVars; i++) {
		auto [lowerBound, upperBound] = relaxedFMIP.getVarBounds(i);
		varRanges[i] = { static_cast<int>(i), upperBound - lowerBound };
	}

	std::sort(varRanges.begin(), varRanges.end(),
			  [](const auto& a, const auto& b) { return a.second < b.second; });

	std::vector<bool> isFixed(numVars, false);
	size_t			  numFixedVars = 0;

	while (!allInteger(rtn.sol) && numFixedVars < numVars) {
		size_t numNotFixedVars = numVars - numFixedVars;
		size_t varsToFix = static_cast<size_t>(numNotFixedVars * theta);

		size_t fixedThisIteration = 0;

		for (size_t i = 0; i < numVars && fixedThisIteration < varsToFix; i++) {
			int idx = varRanges[i].first;
			if (!isFixed[idx]) {
				auto [lowerBound, upperBound] = relaxedFMIP.getVarBounds(idx);

				double clampedLower = std::max(-MAX_UB, lowerBound);
				double clampedUpper = std::min(MAX_UB, upperBound);

				rtn.sol[idx] = rnd.Int(clampedLower, clampedUpper);
				isFixed[idx] = true;
				numFixedVars++;
				fixedThisIteration++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("FixPolicy::firstThetaFixing - %zu vars hard-fixed", varsToFix);
#endif

		std::vector<double> tmp(rtn.sol);
		tmp.resize(relaxedFMIP.getNumCols(), CPX_INFBOUND);
		relaxedFMIP.setVarsValues(tmp);
		relaxedFMIP.solveRelaxation();

		std::vector<double> lpSol = relaxedFMIP.getSol();
		for (size_t i = 0; i < lpSol.size(); ++i) {
			if (isInteger(lpSol[i])) {
				rtn.sol[i] = lpSol[i];
				isFixed[i] = true;
				numFixedVars++;
			}
		}
	}

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
		size_t index {(start + i) % sol.size()};
		model.setVarValue(index,sol[index]);
	}
}
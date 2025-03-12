#include "../include/FixPolicy.hpp"

#define MAX_UB 1e7
#define SPARSE_THRESHOLD 4
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

void FixPolicy::firstThetaFixing(FMIP& fMIP, std::vector<double>& x, double theta, double timeLimit) {
	if (theta < EPSILON || theta >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Theta par. must be within (0,1)!");

	const size_t numVars = x.size();

	if (numVars != fMIP.getMIPNumVars())
		throw FixPolicyException(FPEx::InputSizeError, "Wrong vector size!");

	std::vector<std::pair<int, double>> varRanges(numVars);
	for (size_t i = 0; i < numVars; i++) {
		auto [lowerBound, upperBound] = fMIP.getVarBounds(i);
		varRanges[i] = { static_cast<int>(i), upperBound - lowerBound };
	}

	std::sort(varRanges.begin(), varRanges.end(),
			  [](const auto& a, const auto& b) { return a.second < b.second; });

	std::vector<bool> isFixed(numVars, false);
	size_t			  numFixedVars = 0;

	while (!allInteger(x) && numFixedVars < numVars) {
		size_t numNotFixedVars = numVars - numFixedVars;
		size_t varsToFix = static_cast<size_t>(numNotFixedVars * theta);

		size_t fixedThisIteration = 0;

		for (size_t i = 0; i < numVars && fixedThisIteration < varsToFix; i++) {
			int idx = varRanges[i].first;
			if (!isFixed[idx]) {
				auto [lowerBound, upperBound] = fMIP.getVarBounds(idx);

				double clampedLower = std::max(-MAX_UB, lowerBound);
				double clampedUpper = std::min(MAX_UB, upperBound);

				x[idx] = Random::Int(clampedLower, clampedUpper);
				isFixed[idx] = true;
				numFixedVars++;
				fixedThisIteration++;
			}
		}

#if ACS_VERBOSE >= VERBOSE
		PRINT_INFO("FixPolicy::firstThetaFixing - %zu vars hard-fixed", varsToFix);
#endif

		std::vector<double> tmp(x);
		tmp.resize(fMIP.getNumCols(), CPX_INFBOUND);
		fMIP.setVarsValues(tmp);
		fMIP.solveRelaxation(timeLimit);

		std::vector<double> lpSol = fMIP.getSol();
		for (size_t i = 0; i < lpSol.size(); ++i) {
			if (isInteger(lpSol[i])) {
				x[i] = lpSol[i];
				isFixed[i] = true;
				numFixedVars++;
			}
		}
	}
}

std::vector<size_t> FixPolicy::randomRhoFix(std::vector<double>& x, double rho, const int cpu, const char* type) {
	if (rho < EPSILON || rho >= 1.0)
		throw FixPolicyException(FPEx::InputSizeError, "Rho par. must be within (0,1)!");

	const size_t numVars = x.size();

	const size_t numFixedVars = static_cast<size_t>(rho * numVars);
	const size_t start = Random::Int(0, numVars - 1);

#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("Proc: %3d [%s] - FixPolicy::randomRhoFix - %zu vars hard-fixed", cpu, type, numFixedVars);
#endif
	std::vector<size_t> rtn;

	for (size_t i{ 0 }; i < numFixedVars; i++) {
		rtn.emplace_back((start + i) % numVars);
	}

	return rtn;
}
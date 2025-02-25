#include "../include/FixPolicy.hpp"

const double FixPolicy::MAX_UB = 1e7;

bool isInteger(double n) {
	return ((std::floor(n) == n) && (n != CPX_INFBOUND));
}

bool allInteger(std::vector<double>& x) {
	for (auto e : x)
		if (!isInteger(e))
			return false;
	return true;
}

void FixPolicy::firstThetaFixing(FMIP& fMIP, std::vector<double>& x, double topPerc) {
	if (topPerc < EPSILON || topPerc >= 1.0)
		Logger::print(ERROR, "wrong percentage!");

	std::vector<int> sorter(x.size());
	std::iota(sorter.begin(), sorter.end(), 0);

	if (sorter.size() != x.size())
		Logger::print(ERROR, "sometihing went wrong on init!");

	std::sort(sorter.begin(), sorter.end(), [&fMIP](const int& a, const int& b) {
		auto [aLowerBound, aUpperBound]{ fMIP.getVarBounds(a) };
		auto [bLowerBound, bUpperBound]{ fMIP.getVarBounds(b) };
		return (aUpperBound - aLowerBound) < (bUpperBound - bLowerBound);
	});

	std::set<int> fixedVars;

	while (!allInteger(x) && fixedVars.size() < x.size()) {
		int numNotFixedVars = x.size() - fixedVars.size();
		int varsToFix = static_cast<int>(numNotFixedVars * topPerc);

		for (int i = 0, n = 0; n < varsToFix && i < sorter.size(); i++) {
			int idx = sorter[i];
			if (!fixedVars.count(idx)) {
				auto [lowerBound, upperBound] = fMIP.getVarBounds(idx);
				x[idx] = RandNumGen::randInt(std::max(-FixPolicy::MAX_UB, lowerBound),
											 std::min(FixPolicy::MAX_UB, upperBound));
				fixedVars.insert(idx);
				n++;
			}
		}

		std::vector<double> tmp(x);
		tmp.resize(fMIP.getNumCols(), CPX_INFBOUND);
		fMIP.setVarsValues(tmp);
		fMIP.solveRelaxation(CPX_INFBOUND);
		std::vector<double> lpSol = fMIP.getSol();

		for (size_t i = 0; i < lpSol.size(); i++) {
			if (isInteger(lpSol[i])) {
				x[i] = lpSol[i];
				fixedVars.insert(i);
			}
		}
	}
}

void FixPolicy::randomRhoFix(std::vector<double>& x, double rho) {
	if (rho < EPSILON || rho >= 1.0)
		Logger::print(ERROR, "wrong percentage!");

	int start = RandNumGen::randInt(0, x.size() - 1);
	int numFixedVars{ static_cast<int>(rho * x.size()) };
	int end{ start + numFixedVars - 1 };

#if ACS_VERBOSE == DEBUG
	Logger::print(INFO, "Fixing %d vars starting from %d, in a circular fashion", numFixedVars, start);
#endif

	for (size_t i = 0; i < x.size(); i++)
		if (i < start || i > end)
			x[i] = CPX_INFBOUND;
}
#include "../include/MergePolicy.hpp"

using MPEx = MergePolicy::MergePolicyException::ExceptionType;

std::vector<size_t> MergePolicy::recombine(const std::vector<Solution>& x, const char* phase) {
	if(x.empty())
		throw MergePolicyException(MPEx::InputSizeError, "Empty vector passed to function");

	std::vector<size_t> rtn;

	for (size_t i{ 0 }; i < x[0].sol.size(); i++) {
		if (abs(x[0].sol[i] - x[1].sol[i]) >= EPSILON)
			continue;

		bool commonValue{ true };
		for (size_t p{ 0 }; p < x.size() - 1; p++) {
			if (abs(x[p].sol[i] - x[p + 1].sol[i]) >= EPSILON) {
				commonValue = false;
				break;
			}
		}
		if (commonValue)
			rtn.emplace_back(i);
	}
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("[%s] - MergePolicy::recombine - %zu common vars", phase, rtn.size());
#endif
	return rtn;
}
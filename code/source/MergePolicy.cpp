#include "../include/MergePolicy.hpp"

using MPEx = MergePolicy::MergePolicyException::ExceptionType;

void MergePolicy::recombine(MIP& model, const std::vector<Solution>& x, const char* phase){
	if (x.empty())
		throw MergePolicyException(MPEx::InputSizeError, "Empty vector passed to function");

#if ACS_VERBOSE>=VERBOSE
	size_t numCommVars{0}; 
#endif

	size_t xLen{static_cast<size_t>(model.getMIPNumVars())};
	
	for (size_t i{ 0 }; i < xLen; i++) {
		if (abs(x[0].sol[i] - x[1].sol[i]) >= EPSILON)
			continue;

		bool commonValue{ true };
		for (size_t p{ 0 }; p < x.size() - 1; p++) {
			if (abs(x[p].sol[i] - x[p + 1].sol[i]) >= EPSILON) {
				commonValue = false;
				break;
			}
		}
		if (commonValue){
			model.setVarValue(i,x[0].sol[i]);
#if ACS_VERBOSE>=VERBOSE
			numCommVars++;
#endif
		}
			
	}
	
#if ACS_VERBOSE >= VERBOSE
	PRINT_INFO("[%s] - MergePolicy::recombine - %zu common vars", phase, numCommVars);
#endif

}
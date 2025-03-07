#include "../include/MergePolicy.hpp"

using MPEx = MergePolicy::MergePolicyException::ExceptionType;

std::vector<std::pair<int,double>> MergePolicy::recombine(std::vector<double>& x, int numProc){
    std::vector<std::pair<int,double>> rtn;
    int subArraysLenght{static_cast<int>(x.size()/numProc)};
    for(size_t i{0}; i< subArraysLenght; i++){
        bool commonValue {true};
        for(size_t p{1};p<numProc;p++){
            if(abs(x[i]-x[i+p*subArraysLenght]) >= EPSILON){
                commonValue= false;
                break;
            }
        }
        if(commonValue) rtn.emplace_back(i,x[i]);
    }
#if ACS_VERBOSE >= VERBOSE
Logger::print(Logger::LogLevel::INFO, "MergePolicy::recombine - %zu common vars",rtn.size());
#endif
    return rtn;
}
#ifndef RlxFMIP_H
#define RlxFMIP_H

#include "FMIP.hpp"

using namespace Utils;

class RlxFMIP : public FMIP {

public:
    RlxFMIP(std::string fileName);
    RlxFMIP(const RlxFMIP& otherRlxFMIP) = delete;
	RlxFMIP(const MIP& otherMIP) = delete;
	RlxFMIP& operator=(const MIP&) = delete;
	RlxFMIP& operator=(const RlxFMIP&) = delete;

    int	solve(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);
    int solveRelaxation(const double timeLimit=CPX_INFBOUND);
private:
	std::vector<char> restoreVarType;

	void changeProbType(const int type);

};

#endif
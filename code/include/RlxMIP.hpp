#ifndef RLXMIP_H
#define RLXMIP_H

#include "FMIP.hpp"

using namespace Utils;

class RlxMIP : public FMIP {

public:
    RlxMIP(std::string fileName);
    RlxMIP(const RlxMIP& otherRlxMIP) = delete;
	RlxMIP(const MIP& otherMIP) = delete;
	RlxMIP& operator=(const MIP&) = delete;
	RlxMIP& operator=(const RlxMIP&) = delete;

    int	solve(const double timeLimit = CPX_INFBOUND, const double detTimeLimit = CPX_INFBOUND);
    int solveRelaxation(const double timeLimit=CPX_INFBOUND);
private:
	std::vector<char> restoreVarType;

	void changeProbType(const int type);

};

#endif
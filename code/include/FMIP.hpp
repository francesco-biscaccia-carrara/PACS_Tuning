#ifndef FMIP_H
#define FMIP_H

#include "MIP.hpp"

using namespace Utils;

class FMIP : public MIP {

public:
	FMIP(std::string fileName);
	FMIP(const FMIP& otherFMIP);
	FMIP(const MIP& otherMIP);
	FMIP& operator=(const MIP&) = delete;
	FMIP& operator=(const FMIP&) = delete;

	[[nodiscard]]
	std::vector<double> getSol();

	[[nodiscard]]
	int getMIPNumVars() { return MIPNumVars; };

private:
	void setup();
	int	 MIPNumVars;
};

#endif
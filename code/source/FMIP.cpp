#include "../include/FMIP.hpp"

#define FMIP_SLACK_OBJ_COEFF 1
#define FMIP_VAR_OBJ_COEFF 0

FMIP::FMIP(const std::string fileName) : MIP(fileName) {
	MIPNumVars = getNumCols();
	setup();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_FMIP";
#endif
}

FMIP::FMIP(const FMIP& otherFMIP) : MIP(otherFMIP) {
	MIPNumVars = getNumCols();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_FMIP";
#endif
}

FMIP::FMIP(const MIP& otherMIP) : MIP(otherMIP) {
	MIPNumVars = getNumCols();
	setup();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_FMIP";
#endif
}

void FMIP::setup() {
	std::vector<double> obj(getNumCols(), FMIP_VAR_OBJ_COEFF);
	setObjFunction(obj);

	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i, 1, FMIP_SLACK_OBJ_COEFF, 0, CPX_INFBOUND, "SP_" + std::to_string(i + 1));
	}

	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i, -1, FMIP_SLACK_OBJ_COEFF, 0, CPX_INFBOUND, "SN_" + std::to_string(i + 1));
	}
}
#include "../include/OMIP.hpp"

#define OMIP_SLACK_OBJ_COEFF 0
#define OMIP_BUD_CONST_SENSE 'L'

using MIPEx = MIPException::ExceptionType;

OMIP::OMIP(const std::string fileName) : MIP(fileName) {
	MIPNumVars = getNumCols();
	setup();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_OMIP";
#endif
}

OMIP::OMIP(const OMIP& otherOMIP) : MIP(otherOMIP) {
	MIPNumVars = getNumCols();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_OMIP";
#endif
}

OMIP::OMIP(const MIP& otherMIP) : MIP(otherMIP) {
	MIPNumVars = getNumCols();
	setup();

#if ACS_VERBOSE == DEBUG
	this->fileName += "_OMIP";
#endif
}

OMIP& OMIP::updateBudgetConstr(double rhs) {
	removeRow(getNumRows() - 1);
	addBudgetConstr(rhs);
	return *this;
}

double OMIP::getSlackSum() {
	std::vector<double> xStar = getSol();
	/// FIXED: Bug#5c77b0d838cf9df00715d2bae81ef822eb7ddbd5  -- Unexpected cast to int if init = 0.
	double sum = std::accumulate(xStar.begin() + getMIPNumVars(), xStar.end(), 0.0);
	return sum;
}

void OMIP::setup() {
	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i, 1, OMIP_SLACK_OBJ_COEFF, 0.0, CPX_INFBOUND, "SP_" + std::to_string(i + 1));
	}

	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i, -1, OMIP_SLACK_OBJ_COEFF, 0.0, CPX_INFBOUND, "SN_" + std::to_string(i + 1));
	}

	/// CHANGED: v1.2.2 -- Not necessary 
	/// 	|
	/// 	|
	/// 	v
	addBudgetConstr(CPX_INFBOUND); 
}

void OMIP::addBudgetConstr(double rhs) {
	std::vector<double> budConstr(getNumCols(), 0);
	size_t					start{ getNumCols() - 2 * getNumRows() };
	for (size_t i{start}; i < getNumCols(); i++)
		budConstr[i] = 1;
	addRow(budConstr, OMIP_BUD_CONST_SENSE, rhs);
}

#include "../include/OMIP.hpp"

#define OMIP_SLACK_OBJ_COEFF 0
#define OMIP_BUD_CONST_SENSE 'L'

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
	std::vector<double> xStar = MIP::getSol();
	return std::accumulate(xStar.begin()+getMIPNumVars(), xStar.end(), 0);
}

std::vector<double> OMIP::getSol() {
	std::vector<double> xStar = MIP::getSol();
	xStar.resize(getNumCols() - 2 * (getNumRows() - 1));
	return xStar;
}

void OMIP::setup() {
	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i,1, OMIP_SLACK_OBJ_COEFF, 0., CPX_INFBOUND, "SP_" + std::to_string(i + 1));
	}

	for (size_t i{ 0 }; i < getNumRows(); i++) {
		addCol(i,-1, OMIP_SLACK_OBJ_COEFF, 0., CPX_INFBOUND, "SN_" + std::to_string(i + 1));
	}

	addBudgetConstr(CPX_INFBOUND);
}

void OMIP::addBudgetConstr(double rhs) {
	std::vector<double> budConstr(getNumCols(), 0);
	int					start{ getNumCols() - 2 * getNumRows() };
	for (size_t i{ static_cast<size_t>(start) }; i < getNumCols(); i++)
		budConstr[i] = 1;
	addRow(budConstr, OMIP_BUD_CONST_SENSE, rhs);
}

#ifndef FIX_POL_H
#define FIX_POL_H

#include "FMIP.hpp"

using namespace Utils;

namespace FixPolicy {

    void firstThetaFixing(FMIP& fMIP, std::vector<double>& x,double topPerc);
    void randomRhoFix(std::vector<double>& x,double rho);
};

#endif
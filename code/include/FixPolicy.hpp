#ifndef FIX_POL_H
#define FIX_POL_H

#include "FMIP.hpp"

class FixPolicy {
    
    public:
        static void firstThetaFixing(FMIP& fMIP, std::vector<double>& x,double topPerc);
        static void randomRhoFix(std::vector<double>& x,double rho);

    private:
        const static double MAX_UB;
};

#endif
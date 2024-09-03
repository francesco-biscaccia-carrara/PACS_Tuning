#ifndef FIX_POL_H
#define FIX_POL_H

#include "FMIP.hpp"

class FixPolicy {
    
    public:
        static void firstThetaFixing(FMIP& fmip, std::vector<double>& x,double topPerc);

    private:
        const static double MAX_UB;
        int static randInt(int lb,int ub);
};

#endif
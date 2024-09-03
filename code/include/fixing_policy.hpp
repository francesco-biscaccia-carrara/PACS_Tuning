#ifndef FIX_POL_H
#define FIX_POL_H

#include "FMIP.hpp"

class fixing_policy {
public:
    static void  first_theta_var(FMIP& fmip, std::vector<double>& x,double top_percentage);
};

#endif
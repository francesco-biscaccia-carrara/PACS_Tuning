#ifndef FMIP_H
#define FMIP_H

#include "MIP.hpp"

class FMIP : public MIP {

    public:
        explicit FMIP(std::string fileName);
        explicit FMIP(const FMIP& otherFMIP);
        explicit FMIP(const MIP& otherMIP);

        void saveModel();

        std::vector<double> getSol();
        
    private: 
        const int FMIP_SLACK_OBJ_COEFF = 1;
        const int FMIP_VAR_OBJ_COEFF = 0;

        void setup();
};

#endif
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
        void setup();
};

#endif
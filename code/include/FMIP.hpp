#ifndef FMIP_H
#define FMIP_H

#include "MIP.hpp"

class FMIP : public MIP {

    public:
        FMIP(std::string fileName);
        FMIP(const FMIP& otherFMIP);
        FMIP(const MIP& otherMIP);
        FMIP& operator=(const MIP&) = delete;
        FMIP& operator=(const FMIP&) = delete;

        std::vector<double> getSol();

        #if ACS_VERBOSE == DEBUG
            inline void saveModel() {CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+"_FMIP.lp").c_str(), NULL);};
        #endif

        
    private: 
        void setup();
};

#endif
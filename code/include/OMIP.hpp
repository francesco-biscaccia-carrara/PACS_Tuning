#ifndef OMIP_H
#define OMIP_H

#include "MIP.hpp"

using namespace Utils;

class OMIP : public MIP {

    public:
        OMIP(std::string fileName);
        OMIP(const OMIP& otherOMIP);
        OMIP(const MIP& otherMIP);
        OMIP& operator=(const MIP&) = delete;
        OMIP& operator=(const OMIP&) = delete;

        OMIP& updateBudgetConstr(double rhs);

        std::vector<double> getSol();

        #if ACS_VERBOSE == DEBUG
            inline void saveModel() {CPXwriteprob(env, model, (MIP_LOG_DIR+fileName+"_OMIP.lp").c_str(), NULL);};
        #endif


    private: 
        void setup();
        OMIP& addBudgetConstr(double rhs);

};

#endif
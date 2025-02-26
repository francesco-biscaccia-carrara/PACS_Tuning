#ifndef OMIP_H
#define OMIP_H

#include "MIP.hpp"

using namespace Utils;

class OMIP : public MIP {

    public:
        OMIP(const std::string fileName);
        OMIP(const OMIP& otherOMIP);
        OMIP(const MIP& otherMIP);
        OMIP& operator=(const MIP&) = delete;
        OMIP& operator=(const OMIP&) = delete;

        OMIP& updateBudgetConstr(double rhs);

        [[nodiscard]] std::vector<double> getSol();
        
    private: 
        void setup();
        OMIP& addBudgetConstr(double rhs);

};

#endif
#ifndef OMIP_H
#define OMIP_H

#include "MIP.hpp"

class OMIP : public MIP {

    public:
        explicit OMIP(std::string fileName);
        explicit OMIP(const OMIP& otherOMIP);
        explicit OMIP(const MIP& otherMIP);

        void saveModel();

        void updateBudgetConstr(double rhs);

        std::vector<double> getSol();

    private: 
        void setup();
        void addBudgetConstr(double rhs);

};

#endif
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
        const int OMIP_SLACK_OBJ_COEFF = 0;
        const char OMIP_BUD_CONST_SENSE = 'L';
        
        void setup();
        void addBudgetConstr(double rhs);

};

#endif
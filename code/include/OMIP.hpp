#ifndef OMIP_H
#define OMIP_H

#include "MIP_solver.hpp"

class OMIP : public MIP_solver {

    public:
        explicit OMIP(std::string file_name);
        explicit OMIP(const OMIP& other_solver);
        explicit OMIP(const MIP_solver& other_solver);

        void save_model();

        void update_budget_constraint(double rhs);

        std::vector<double> get_solution();

    private: 
        void prepare();
        void add_budget_constraint(double rhs);

};

#endif
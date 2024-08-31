#ifndef MIP_SOL_H
#define MIP_SOL_H

#include "utils.hpp"
#include <cplex.h>

#define CPLEX_LOG_DIR "../log/cplex_out/log/"
#define MIP_LOG_DIR "../log/cplex_out/mip/"
#define INST_DIR "../instances/"

class MIP_solver{

    public:
        explicit MIP_solver(std::string file_name);
        explicit MIP_solver(const MIP_solver& other_solver);
        
        int solve(double time_limit);
        double get_obj_value();
        std::vector<double> get_solution();
        //void change_obj();
        //void change_bounds();
        //void add_variable();
        //void remove_variable();
        //void add_constraint();
        //void remove_constraint();

        ~MIP_solver();

    private:
        std::string file_name;
        CPXLPptr model; 
        CPXENVptr env;
};


#endif
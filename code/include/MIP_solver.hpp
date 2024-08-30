#ifndef MIP_SOL_H
#define MIP_SOL_H

#include <cplex.h>
#include <string>

class MIP_solver{

    public:
        explicit MIP_solver(std::string file_name);
        explicit MIP_solver(const MIP_solver& other_solver);
        
        int solve(double time_limit);
        void change_obj();
        void change_bounds();
        void add_variable();
        void remove_variable();
        void add_constraint();
        void remove_constraint();

        ~MIP_solver();

    private:
        CPXLPptr model; 
        CPXENVptr env;

        void new_model(std::string file_name);
        void delete_model();
        
};


#endif
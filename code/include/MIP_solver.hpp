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
        
        void set_num_processors(int num_procs);
        int solve(double time_limit);
        void save_model();
        
        double get_obj_value();
        std::vector<double> get_solution();

        int get_num_cols();
        int get_num_rows();

        void add_col(std::vector<double>& new_col, double obj_coef, double lb, double ub, std::string name);
        void add_row(std::vector<double>& new_row, char sense, double rhs);
        void remove_row(int index);

        std::vector<double> get_obj_function();
        void set_obj_function(std::vector<double> new_obj);

        std::pair<double,double> get_var_bounds(int index);
        void set_var_value(int index, double val);
        void set_vars_value(std::vector<double>& values);

        ~MIP_solver();

    protected:
        std::string file_name;
        CPXLPptr model; 
        CPXENVptr env;
};


#endif
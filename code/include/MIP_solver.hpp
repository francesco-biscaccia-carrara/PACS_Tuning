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
        void save_model();
        
        double get_obj_value();
        std::vector<double> get_solution();

        int get_num_cols();
        int get_num_rows();
        int get_num_vars();

        void add_col(std::vector<double> new_col, double obj_coef, double lb, double ub, std::string name);
        void add_row(std::vector<double> new_row, char sense, double rhs);

        std::vector<double> get_obj_function();
        void set_obj_function(std::vector<double> new_obj);

        void set_var_value(int index, double val);

        ~MIP_solver();

    private:
        std::string file_name;
        CPXLPptr model; 
        CPXENVptr env;
};


#endif
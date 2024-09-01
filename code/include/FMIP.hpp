#ifndef FMIP_H
#define FMIP_H

#include "MIP_solver.hpp"

class FMIP : public MIP_solver {

    public:
        explicit FMIP(std::string file_name);
        explicit FMIP(const FMIP& other_solver);
        
        std::vector<double> get_old_obj_function();
        int get_num_og_var();
        void prepare();

    private:
        int num_og_var;
        std::vector<double> old_obj;
    
};

#endif
#ifndef FMIP_H
#define FMIP_H

#include "MIP_solver.hpp"

class FMIP : public MIP_solver {

    public:
        explicit FMIP(std::string file_name);
        explicit FMIP(const FMIP& other_solver);
        explicit FMIP(const MIP_solver& other_solver);

        void save_model();

        std::vector<double> get_solution();
        
    private: 
        void prepare();
};

#endif
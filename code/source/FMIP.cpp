#include "../include/FMIP.hpp"

FMIP::FMIP(std::string file_name) : MIP_solver(file_name){
    old_obj = get_obj_function();
    num_og_var = get_num_vars();
}

FMIP::FMIP(const FMIP& other_solver) : MIP_solver(other_solver){
    old_obj = other_solver.old_obj;
    num_og_var = other_solver.num_og_var;
}

std::vector<double> FMIP::get_old_obj_function() {return old_obj;}
int FMIP::get_num_og_var(){return num_og_var;}

void FMIP::prepare(){
    std::vector<double> obj(get_num_vars(),0);
    set_obj_function(obj);

    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=1;
        add_col(col,1,0,CPX_INFBOUND,"SP_"+std::to_string(i+1));
    }

    for(int i=0;i<get_num_rows();i++){
        std::vector<double> col(get_num_rows(),0);
        col[i]=-1;
        add_col(col,1,0,CPX_INFBOUND,"SN_"+std::to_string(i+1));
    }

    //Set some var.
}
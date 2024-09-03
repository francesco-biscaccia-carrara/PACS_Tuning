#include "../include/fixing_policy.hpp"

#define MAX_UB 1e7


int static random_integer(int lb,int ub){
    return (int) (lb + std::rand()%(ub-lb+1));
}

void fixing_policy::first_theta_var(FMIP& fmip, std::vector<double>& x,double top_percentage){
    if(top_percentage < EPSILON || top_percentage > 1.0) print_state(ERROR,"wrong percentage!");

    int x_length = x.size();
    std::vector<std::pair<int,double>> sorter;
    for(int i = 0 ;i < x_length; i++){
        std::pair<double,double> vb = fmip.get_var_bounds(i);
        std::pair<int,double> p{i,vb.second-vb.first};
        sorter.push_back(p);
    }

    if(sorter.size()!= x_length) print_state(ERROR,"sometihing went wrong on init!");

    std::sort(sorter.begin(),sorter.end(),  [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
    });
    sorter.resize((int)(top_percentage*x_length));

    for(auto e: sorter){
        std::pair<double,double> vb = fmip.get_var_bounds(e.first);
        x[e.first]=random_integer(std::max(-MAX_UB,vb.first),std::min(MAX_UB,vb.second));
    }
}

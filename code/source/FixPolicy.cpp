#include "../include/FixPolicy.hpp"

const double FixPolicy::MAX_UB = 1e7;


int FixPolicy::randInt(int lb,int ub){
    return (int) (lb + std::rand()%(ub-lb+1));
}

void FixPolicy::firstThetaFixing(FMIP& fmip, std::vector<double>& x,double topPerc){
    if(topPerc < EPSILON || topPerc > 1.0) Logger::print(ERROR,"wrong percentage!");

    int xLength = x.size();
    std::vector<std::pair<int,double>> sorter;
    for(int i = 0 ;i < xLength; i++){
        std::pair<double,double> vb = fmip.getVarBounds(i);
        std::pair<int,double> p{i,vb.second-vb.first};
        sorter.push_back(p);
    }

    if(sorter.size()!= xLength) Logger::print(ERROR,"sometihing went wrong on init!");

    std::sort(sorter.begin(),sorter.end(),  [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
    });
    sorter.resize((int)(topPerc*xLength));

    for(auto e: sorter){
        std::pair<double,double> vb = fmip.getVarBounds(e.first);
        x[e.first]=randInt(std::max(-FixPolicy::MAX_UB,vb.first),std::min(FixPolicy::MAX_UB,vb.second));
    }
}

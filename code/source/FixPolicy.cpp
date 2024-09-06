#include "../include/FixPolicy.hpp"

const double FixPolicy::MAX_UB = 1e7;

bool allInteger(std::vector<double>& x){
    for(auto e : x) if((std::floor(e) != e) || (e == CPX_INFBOUND)) return false;
    return true;
}

void FixPolicy::firstThetaFixing(FMIP& fMIP, std::vector<double>& x,double topPerc){
    if(topPerc < EPSILON || topPerc > 1.0) Logger::print(ERROR,"wrong percentage!");

    int xLength = x.size();
    std::vector<std::pair<int,double>> sorter;
    for(int i = 0 ;i < xLength; i++){
        std::pair<double,double> vb = fMIP.getVarBounds(i);
        std::pair<int,double> p{i,vb.second-vb.first};
        sorter.push_back(p);
    }

    if(sorter.size()!= xLength) Logger::print(ERROR,"sometihing went wrong on init!");

    std::sort(sorter.begin(),sorter.end(),  [](const std::pair<int, double>& a, const std::pair<int, double>& b) {
        return a.second < b.second;
    });
    
    //FIXME: error here and below   
    while(!allInteger(x)){
        for(int i=0;i<(int)(topPerc*xLength);i++){
            std::pair<double,double> vb = fMIP.getVarBounds(sorter[i].first);
            x[sorter[i].first]=RandNumGen::randInt(std::max(-FixPolicy::MAX_UB,vb.first),std::min(FixPolicy::MAX_UB,vb.second));
        } 
        sorter.erase(sorter.begin(),sorter.begin()+(topPerc*xLength));

        std::vector<double> tmp (x);
        tmp.resize(fMIP.getNumCols(),CPX_INFBOUND);
        fMIP.setVarsValues(tmp);
        fMIP.solveRelaxation(100.0);
        std::vector<double> lpSol = fMIP.getSol();
        for(int i=0;i<lpSol.size();i++){
            if(std::floor(lpSol[i]) == lpSol[i] && std::floor(x[i]) != lpSol[i]){
                x[i]=lpSol[i];
                sorter.erase(sorter.begin()+i,sorter.begin()+i+1);
            }
        }
    }
    

}

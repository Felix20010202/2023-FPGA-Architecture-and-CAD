#include "DetailedPlacer.h"
#include "FPGA.h"

#include <vector>
#include <climits>
#include <algorithm>
#include <iostream>
#include <chrono>
using namespace std;

DetailedPlacer::DetailedPlacer(FPGA::Placement &placement, bool &tout)
    : p(placement), timeOut(tout)
{
}

NetCost::NetCost(double c, double Xu, double Xb, double Yu, double Yb){
    cost = c;
    Xmax = Xu;
    Xmin = Xb;
    Ymax = Yu;
    Ymin = Yb;
}

void DetailedPlacer::place(bool &timeOut){
    initPlace();

    int passCnt = 0;
    double originWL = p.getHPWL();
    double newWL = originWL-1;
    int cnt = 0;

    // localMaskSize = p.architecture.size()*0.025;

    while(timeOut == false){
        cnt = 0;
        while(originWL-newWL > 0 && timeOut == false){
            originWL = newWL;

            getPriority();

            for(unsigned int i = 0; i < p.instanceNum; i++){
                if(timeOut == true){
                    break;
                }
                globalMove(priority[i].first);
                localMove(priority[i].first);
            }

            newWL = p.getHPWL();
            double gain = originWL-newWL;

            if(gain > originWL/100){
                cout<<"Iteration "<<passCnt<<": Reduce the length by "<<(long long)gain<<", new HPWL: "<<(long long)newWL<<endl;
                cnt++;
                passCnt++;
            }
            else if(gain > 0){
                cout<<"Iteration "<<passCnt<<": Reduce the length by "<<(long long)gain<<", new HPWL: "<<(long long)newWL<<endl;
                cnt++;
                passCnt++;
                break;
            }
        }
        localMaskSize += (p.archWidth/20);
        newWL = originWL-1;

        if(cnt == 0){
            break;
        }
    }
}

void DetailedPlacer::initPlace(){
    for(unsigned int i = 0; i < p.netNum; i++){
        double Xmax = INT_MIN, Xmin = INT_MAX;
        double Ymax = INT_MIN, Ymin = INT_MAX;
        for(unsigned int j = 0; j < p.net[i].netSize; j++){
            unsigned int idx = p.net[i].pinIns[j];
            if(Xmax < p.instance[idx].x){
                Xmax = p.instance[idx].x;
            }
            if(Xmin > p.instance[idx].x){
                Xmin = p.instance[idx].x;
            }
            if(Ymax < p.instance[idx].y){
                Ymax = p.instance[idx].y;
            }
            if(Ymin > p.instance[idx].y){
                Ymin = p.instance[idx].y;
            }
        }
        double c = (Xmax - Xmin) + (Ymax - Ymin);
        netCost.push_back(NetCost(c, Xmax, Xmin, Ymax, Ymin));
    }
}

double DetailedPlacer::insCost(unsigned int insID, double x, double y){

    double result = 0;
    for(unsigned int i = 0; i < p.instance[insID].netID.size(); i++){
        unsigned int netID = p.instance[insID].netID[i];

        if(x >= netCost[netID].Xmin && x <= netCost[netID].Xmax && y >= netCost[netID].Ymin && y <= netCost[netID].Ymax){
            if(p.instance[insID].x > netCost[netID].Xmin && p.instance[insID].x < netCost[netID].Xmax && p.instance[insID].y > netCost[netID].Ymin && p.instance[insID].y < netCost[netID].Ymax){
                result += netCost[netID].cost;
                continue;
            }
        }

        double Xmax = x, Xmin = x;
        double Ymax = y, Ymin = y;

        for(unsigned int j = 0; j < p.net[netID].netSize; j++){
            if(p.net[netID].pinIns[j] == insID){
                continue;
            }
            unsigned int idx = p.net[netID].pinIns[j];
            if(Xmax < p.instance[idx].x){
                Xmax = p.instance[idx].x;
            }
            if(Xmin > p.instance[idx].x){
                Xmin = p.instance[idx].x;
            }
            if(Ymax < p.instance[idx].y){
                Ymax = p.instance[idx].y;
            }
            if(Ymin > p.instance[idx].y){
                Ymin = p.instance[idx].y;
            }
        }
        result += (Xmax - Xmin) + (Ymax - Ymin);
    }

    return result;
}

double DetailedPlacer::insCost(unsigned int insID){
    double result = 0;
    for(unsigned int i = 0; i < p.instance[insID].netID.size(); i++){
        unsigned int netID = p.instance[insID].netID[i];

        result += netCost[netID].cost;
    }

    return result;
}

void DetailedPlacer::getPriority(){
    for(unsigned int i = 0; i < p.instanceNum; i++){
        priority.push_back(pair<unsigned int, double>(i, insCost(i)));
    }

    sort(priority.begin(), priority.end(), [](pair<unsigned int, double> a, pair<unsigned int, double> b){
        return a.second > b.second;
    });
} 

void DetailedPlacer::localMove(unsigned int insID){
    if(p.instance[insID].InsType == "IO"){
        return;
    }

    unsigned int ax = p.instance[insID].resPos.first;
    unsigned int ay = p.instance[insID].resPos.second;
    
    double gain = 0;
    unsigned int swapX = 0, swapY = 0;

    for(int i = -1*localMaskSize; i <= localMaskSize; i++){
        for(int j = -1*localMaskSize; j <= localMaskSize; j++){
            if(i == 0 && j == 0){
                continue;
            }
            if((int)ax+(int)i < 0 || (int)ay+(int)j < 0){
                continue;
            }
            double swapGain = trySwap(insID, ax+i,ay+j);
            if(swapGain > gain){
                gain = swapGain;
                swapX = ax+i;
                swapY = ay+j;
            }
        }
    }
    
    if(gain > 0){
        swap(insID, swapX, swapY);
    }
}

void DetailedPlacer::globalMove(unsigned int insID){
    if(p.instance[insID].InsType == "IO"){
        return;
    }

    vector<double> netX;
    vector<double> netY;

    int netNum = p.instance[insID].netID.size();

    if(netNum == 0){
        return;
    }

    for(int i = 0; i < netNum; i++){
        unsigned int netID = p.instance[insID].netID[i];
        netX.push_back(netCost[netID].Xmax);
        netX.push_back(netCost[netID].Xmin);
        netY.push_back(netCost[netID].Ymax);
        netY.push_back(netCost[netID].Ymin);
    }

    
    nth_element(netX.begin(), netX.begin()+(netNum/2), netX.end());
    nth_element(netY.begin(), netY.begin()+(netNum/2), netY.end());
    
    double medX = netX[netNum/2];
    double medY = netY[netNum/2];

    unsigned int bx = p.findXPos(medX);
    unsigned int by = p.findYPos(bx, medY);

    double swapGain = trySwap(insID, bx, by);

    if(swapGain > 0){
        swap(insID, bx, by);
    }
}


void DetailedPlacer::swap(unsigned int insID, unsigned int idxX, unsigned int idxY){
    unsigned int originX = p.instance[insID].resPos.first;
    unsigned int originY = p.instance[insID].resPos.second;
    if(p.architecture[idxX][idxY].isMatch == false){
        p.architecture[originX][originY].clear();
        p.architecture[idxX][idxY].isMatch = true;
    }
    else{
        unsigned int bIdx = p.architecture[idxX][idxY].matchInsID;
        p.architecture[originX][originY].matchInsName = p.instance[bIdx].name;
        p.architecture[originX][originY].matchInsID = bIdx;
        p.instance[bIdx].x = p.architecture[originX][originY].x();
        p.instance[bIdx].y = p.architecture[originX][originY].y();
        p.instance[bIdx].finalResource = p.architecture[originX][originY].name;
        p.instance[bIdx].resPos = pair<unsigned int, unsigned int>(originX, originY);
        updateNetCost(bIdx);
    }

    p.architecture[idxX][idxY].matchInsName = p.instance[insID].name;
    p.architecture[idxX][idxY].matchInsID = insID;
    p.instance[insID].x = p.architecture[idxX][idxY].x();
    p.instance[insID].y = p.architecture[idxX][idxY].y();
    p.instance[insID].finalResource = p.architecture[idxX][idxY].name;
    p.instance[insID].resPos = pair<unsigned int, unsigned int>(idxX, idxY);
    updateNetCost(insID);
}

double DetailedPlacer::trySwap(unsigned int insID, unsigned int idxX, unsigned int idxY){
    if(idxX >= p.architecture.size() || idxY >= p.architecture[idxX].size()){
        return INT_MIN;
    }
    if(p.architecture[idxX][0].t  != p.instance[insID].InsType){
        return INT_MIN;
    }

    unsigned int ax = p.instance[insID].resPos.first;
    unsigned int ay = p.instance[insID].resPos.second;
    unsigned int bx = idxX, by = idxY;
    double before = 0, after = 0;
    if(p.architecture[bx][by].isMatch == false){
        before = insCost(insID);
        after = insCost(insID, p.architecture[bx][by].x(), p.architecture[bx][by].y());
    }
    else{
        unsigned int bID = p.architecture[bx][by].matchInsID;
        before = insCost(insID) + insCost(bID);
        after = insCost(insID, p.architecture[bx][by].x(), p.architecture[bx][by].y())
                + insCost(bID, p.architecture[ax][ay].x(), p.architecture[ax][ay].y());
        
    }

    return (before - after);
}

void DetailedPlacer::updateNetCost(unsigned int idx){
    for(unsigned int i = 0; i < p.instance[idx].netID.size(); i++){
        unsigned int netIdx = p.instance[idx].netID[i];
        double Xmax = INT_MIN, Xmin = INT_MAX;
        double Ymax = INT_MIN, Ymin = INT_MAX;
        for(unsigned int j = 0; j < p.net[netIdx].netSize; j++){
            unsigned int insIdx = p.net[netIdx].pinIns[j];
            
            if(Xmax < p.instance[insIdx].x){
                Xmax = p.instance[insIdx].x;
            }
            if(Xmin > p.instance[insIdx].x){
                Xmin = p.instance[insIdx].x;
            }
            if(Ymax < p.instance[insIdx].y){
                Ymax = p.instance[insIdx].y;
            }
            if(Ymin > p.instance[insIdx].y){
                Ymin = p.instance[insIdx].y;
            }
        }
        netCost[netIdx].cost = (Xmax - Xmin) + (Ymax - Ymin);
        netCost[netIdx].Xmax = Xmax;
        netCost[netIdx].Xmin = Xmin;
        netCost[netIdx].Ymax = Ymax;
        netCost[netIdx].Ymin = Ymin;
    }
}
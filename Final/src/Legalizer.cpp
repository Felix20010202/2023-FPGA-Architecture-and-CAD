#include "Legalizer.h"
#include "FPGA.h"

#include <vector>
#include <iostream>
#include <algorithm>
#include <climits>
using namespace std;

Legalizer::Legalizer(FPGA::Placement &placement)
    : placement_(placement) 
{
}

void Legalizer::legal(){
    
    tetris();
    
    // divSubspace();
    // getSpacePriority();
    // for(unsigned int i = 0; i < spaceNum(); i++){
    //     abacus(i);
    // }
}

void Legalizer::Legalizer::divSubspace(){
    vector<pair<unsigned int, unsigned int>> interval;
    
    unsigned int begin = 0, end = 0;
    bool flag = false;

    if(placement_.archWidth > 0 && placement_.specialCol(0) == true){
        flag = true;
    }

    for(unsigned i = 0; i < placement_.archWidth; i++){
        if(flag == false && placement_.specialCol(i) == true){
            flag = true;
        }

        if(flag == true && placement_.specialCol(i) == false){
            flag = false;
            end = i-1;
            bool find0 = false;
            for(unsigned int j = begin; j < i; j++){
                if(placement_.specialCol(j) == false){
                    find0 = true;
                    break;
                }
            }
            if(find0 == true){
                interval.push_back(pair<unsigned int, unsigned int>(begin, end));
                for(int j = end; j >= 0; j--){
                    if(placement_.specialCol(j) == true){
                        begin = j;
                    }
                    else{
                        break;
                    }
                }
            }
        }
    }

    interval.push_back(pair<unsigned int, unsigned int>(begin, placement_.architecture.size()-1));

    // for(unsigned int i = 0; i < placement_.archWidth; i++){
    //     if(placement_.specialCol(i) == true){
    //         cout<<i<<" ";
    //     }
    // }
    // cout<<endl;

    // for(unsigned int i = 0; i < interval.size(); i++){
    //     cout<<interval[i].first<<" "<<interval[i].second<<endl;
    // }
    // vector<Subspace>()
    double bias = placement_.colWidth/2;
    for(unsigned int i = 0; i < interval.size(); i++){
        unsigned int begin = interval[i].first;
        unsigned int end  = interval[i].second;
        int beginX = placement_.architecture[begin][0].x();
        int endX = placement_.architecture[end][0].x();
        subspaces.push_back(Subspace(begin, end, beginX-bias, endX+bias));
    }

    for(unsigned int i = 0; i < placement_.instanceNum; i++){
        int x = placement_.instance[i].x;
        bool find = false;
        for(unsigned int j = 0; j < spaceNum(); j++){
            if(subspaces[j].beginX <= x && x <= subspaces[j].endX){
                subspaces[j].ins.push_back(pair<unsigned int, pair<double,double>>(i, pair<double,double>(placement_.instance[i].x,placement_.instance[i].y)));
                find = true;
                break;
            }
        }

        if(find == false){
            cerr<<"error: Instance can't put into subspace."<<endl;
            subspaces[0].ins.push_back(pair<unsigned int, pair<double,double>>(i, pair<double,double>(0,0)));
        }
    }

    for(unsigned int i = 0; i < subspaces.size(); i++){
        sort(subspaces[i].ins.begin(), subspaces[i].ins.end(), cmp);
    }

    // for(unsigned int i = 0; i < subspaces[0].ins.size(); i++){
    //     cout<<subspaces[0].ins[i].second.first<<" "<<subspaces[0].ins[i].second.second<<endl;
    // }

    // for(unsigned int i = 0; i < placement_.archWidth; i++){
    //     if(placement_.specialCol(i) == false){
    //         continue;
    //     }
    //     for(unsigned int j = 0; j < placement_.architecture[i].size(); j++){
    //         if(placement_.architecture[i][j].t == "CLB"){
    //             cout<<"Find CLB!"<<endl;
    //         }
    //     }
    // }
    // cout<<"Total instance number: "<<placement_.instanceNum<<endl;
    // for(unsigned int i = 0; i < spaceNum(); i++){
    //     cout<<"instance number: "<<subspaces[i].ins.size()<<endl;
    // }
}

unsigned int Legalizer::spaceNum(){
    return subspaces.size();
}

Subspace::Subspace(unsigned int b, unsigned int e, int bx, int ex){
    begin = b;
    end = e;
    beginX = bx;
    endX = ex;
}

void Legalizer::getSpacePriority(){
    if(subspaces.size() == 0){
        cout<<"Warning: No subspaces found.."<<endl;
        return;
    }

    for(unsigned int i = 0; i < spaceNum(); i++){
        spacePriority.push_back(i);
    }
}

bool Legalizer::cmp(pair<unsigned int,pair<double, double>> &a, pair<unsigned int,pair<double, double>>&b){
    if(a.second.first == b.second.first){
        return a.second.second < b.second.second;
    }
    return a.second.first < b.second.first;
}

void Legalizer::abacus(unsigned int idx){
    cout<<idx<<endl;
}

void Legalizer::tetris(){

    vector<pair<unsigned int,pair<double, double>>> ins;
    for(unsigned int i = 0; i < placement_.instanceNum; i++){
        ins.push_back(pair<unsigned int, pair<double,double>>(i, pair<double,double>(placement_.instance[i].x,placement_.instance[i].y)));
    }

    sort(ins.begin(), ins.end(), cmp);

    for(unsigned int i = 0; i < placement_.instanceNum; i++){
        unsigned int insIdx = ins[i].first;
        double x = ins[i].second.first;
        double y = ins[i].second.second;
        string t = placement_.instance[insIdx].InsType;
        if(t == "IO"){
            continue;
        }

        double minMove = INT_MAX;
        int idxX = -1, idxY = -1;
        int startIdxX = placement_.findXPos(x);
        pair<bool, unsigned int> rec = pair<bool, unsigned int>(false, 0);
        if(placement_.architecture[startIdxX][0].t == t){
            rec = findColPutPos(startIdxX, y);
        }
        if(rec.first == true){
            idxX = startIdxX;
            idxY = rec.second;
            minMove = abs(placement_.architecture[idxX][idxY].y() - y);
        }

        int bias = 1;

        while(minMove > (bias-1)*placement_.colWidth){
            if(startIdxX - bias >= 0 && placement_.architecture[startIdxX - bias][0].t == t){
                rec = findColPutPos(startIdxX - bias, y);
                double distance = abs(placement_.architecture[startIdxX - bias][rec.second].y() - y) + abs(placement_.architecture[startIdxX - bias][rec.second].x() - x);
                if(rec.first == true && minMove > distance){
                    idxX = startIdxX - bias;
                    idxY = rec.second;
                    minMove = distance;
                }
            }

            if(((long long)startIdxX + bias) < placement_.archWidth && placement_.architecture[startIdxX + bias][0].t == t){
                rec = findColPutPos(startIdxX + bias, y);
                double distance = abs(placement_.architecture[startIdxX + bias][rec.second].y() - y) + abs(placement_.architecture[startIdxX + bias][rec.second].x() - x);
                if(rec.first == true && minMove > distance){
                    idxX = startIdxX + bias;
                    idxY = rec.second;
                    minMove = distance;
                }
            }

            bias++;
        }
        if(idxY == -1){
            cerr<<"error: tetris can't legalize."<<endl;
            // for(unsigned int i = 0; i < placement_.architecture.size(); i++){
            //     for(unsigned int j = 0; j < placement_.architecture[i].size(); j++){
            //         if(placement_.architecture[i][j].isMatch == false && t == placement_.architecture[i][j].t){
            //             cout<<i<<" "<<j<<" "<<placement_.architecture[i][j].t<<endl;
            //         }
            //     }
            // }
            exit(1);
        }
        else{
            putIns(insIdx, idxX, idxY);
        }
    }
}

void Legalizer::putIns(unsigned int insIdx, unsigned int idxX, unsigned int idxY){
    if(idxX >= placement_.archWidth || idxY >= placement_.architecture[idxX].size()){
        cerr<<"error: Legalizer::putIns"<<endl;
        exit(1);
    }
    
    placement_.architecture[idxX][idxY].isMatch = true;
    placement_.architecture[idxX][idxY].matchInsName = placement_.instance[insIdx].name;
    placement_.architecture[idxX][idxY].matchInsID = insIdx;
    placement_.instance[insIdx].x = placement_.architecture[idxX][idxY].x();
    placement_.instance[insIdx].y = placement_.architecture[idxX][idxY].y();
    placement_.instance[insIdx].finalResource = placement_.architecture[idxX][idxY].name;
    placement_.instance[insIdx].resPos = pair<unsigned int, unsigned int>(idxX, idxY);
}

pair<bool, unsigned int> Legalizer::findColPutPos(unsigned int idx, double y){
    pair<bool, unsigned int> result;
    result.first = false; 
    unsigned int startPos = placement_.findYPos(idx, y);
    double min = INT_MAX;
    unsigned int resIdx = 0;
    
    for(unsigned int i = startPos; i < placement_.architecture[idx].size(); i++){
        if(placement_.architecture[idx][i].isMatch == false){
            if(min > abs(placement_.architecture[idx][i].y() - y)){
                min = abs(placement_.architecture[idx][i].y() - y);
                resIdx = i;
            }
            break;
        }
    }

    for(int i = startPos-1; i >= 0; i--){
        if(placement_.architecture[idx][i].isMatch == false){
            if(min > abs(placement_.architecture[idx][i].y() - y)){
                min = abs(placement_.architecture[idx][i].y() - y);
                resIdx = i;
            }
            break;
        }
    }

    if(min == INT_MAX){
        return pair<bool, unsigned int>(false, 0);
    }

    return pair<bool, unsigned int>(true, resIdx);
}

void Legalizer::adjust(){
    colSorted.resize(placement_.archWidth);
    for(unsigned int i = 0; i < placement_.archWidth; i++){
        sortCol(i);
    }


}

void Legalizer::sortCol(unsigned int idx){
    vector<pair<unsigned int, double>> insY;
    for(unsigned int i = 0; i < placement_.architecture[idx].size(); i++){
        if(placement_.architecture[idx][i].isMatch == true){
            unsigned int insIdx =  placement_.architecture[idx][i].matchInsID;
            double y = placement_.instance[insIdx].y;
            insY.push_back(pair<unsigned int, double>(insIdx, y));
        }
    }

    sort(insY.begin(), insY.end(), [](pair<unsigned int, double>a,pair<unsigned int, double>b){
        return a.second < b.second;
    });

    colSorted[idx].resize(insY.size());
    for(unsigned int i = 0; i < insY.size(); i++){
        colSorted[idx][i] = insY[i].first;
    }
}

void Legalizer::adjustCol(unsigned int idx){
    vector<Cluster> clusters;
    for(unsigned int i = 0; i < colSorted[idx].size(); i++){
        
    }
}
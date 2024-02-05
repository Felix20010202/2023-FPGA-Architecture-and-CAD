#include "FPGA.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
using namespace std;

FPGA::Net::Net(string n){
    name = n;
}

FPGA::Instance::Instance(string n, string t, double argX, double argY){
    name = n;
    InsType = t;
    x = argX;
    y = argY;
}

FPGA::Architecture::Architecture(string n, string argT, double argX, double argY){
    name = n;
    t = argT;
    x_ = argX;
    y_ = argY;
}

double FPGA::Architecture::x(){
    return x_;
}

double FPGA::Architecture::y(){
    return y_;
}

void FPGA::Architecture::clear(){
    isMatch = false;
    matchInsName = "None";
    matchInsID = -1;
}

void FPGA::Placement::readFileInstance(string filePath){
    ifstream fin;
    fin.open(filePath);

    if (!fin.is_open()){
        cerr<<"error: Can't open "<<filePath<<endl;
        exit(1);
    }

    string name, t;
    double x, y;

    while(!fin.eof()){
        if(fin>>name){
            fin>>t>>x>>y;
        }
        else{
            break;
        }

        insMap[name] = instanceNum;
        instanceNum++;
        instance.push_back(FPGA::Instance(name,t,x,y));
    }

    // for(unsigned int i = 0; i < instanceNum; i++){
    //     cout<<instance[i].name<<" "<<instance[i].InsType<<" "<<instance[i].x<<" "<<instance[i].y<<endl;
    // }
    // cout<<"Total Instance Number: "<<instanceNum<<endl;

    fin.close();
}

void FPGA::Placement::readFileNet(string filePath){
    ifstream fin;
    fin.open(filePath);

    if (!fin.is_open()){
        cerr<<"error: Can't open "<<filePath<<endl;
        exit(1);
    }

    string line;
    string name, InsName;
    while(std::getline(fin, line)){
        istringstream lineStream(line);
        lineStream>>name;
        netMap[name] = netNum;

        string insName;
        unsigned int insID;
        net.push_back(FPGA::Net(name));

        while(!lineStream.eof()){
            lineStream>>insName;
            insID = insMap[insName];
            net[netNum].pinIns.push_back(insID);
            net[netNum].insName.push_back(insName);
        }

        net[netNum].netSize = net[netNum].pinIns.size();
        netNum++;
    }
    

    // for(unsigned int i = 0; i < netNum; i++){
    //     cout<<net[i].name<<" ";
    //     for(unsigned int j = 0; j < net[i].pinIns.size(); j++){
    //         cout<<net[i].insName[j]<<" ";
    //     }
    //     cout<<net[i].netSize<<endl;
    // }

    fin.close();
}

void FPGA::Placement::readFileArch(string filePath){
    ifstream fin;
    fin.open(filePath);

    if (!fin.is_open()){
        cerr<<"error: Can't open "<<filePath<<endl;
        exit(1);
    }

    string name, t;
    double x, y;
    int idx = -1;

    double columnX = INT_MIN;
    bool special = false;
    while(!fin.eof()){
        if(fin>>name){
            fin>>t>>x>>y;
        }
        else{
            break;
        }
        if(x != columnX){
            idx++;
            columnX = x;
            architecture.push_back(vector<Architecture>());
        }

        architecture[idx].push_back(Architecture(name,t,x,y));
    }

    for(unsigned int i = 0; i < architecture.size(); i++){
        special = false;
        for(unsigned int j = 0; j < architecture[i].size(); j++){
            if(architecture[i][j].t != "CLB"){
                special = true;
                break;
            }
        }
        specialCol_.push_back(special);
    }

    // int cnt = 0;
    // for(unsigned int i = 0; i < architecture.size(); i++){
    //     for(unsigned int j = 0; j < architecture[i].size(); j++){
    //         cout<<architecture[i][j].name<<" "<<architecture[i][j].t<<" "<<architecture[i][j].x()<<" "<<architecture[i][j].y()<<endl;
    //         cnt++;
    //     }
    // }
    // cout<<"cnt: "<<cnt<<endl;

    // cout<<architecture.size()<<endl;
    // for(unsigned int i = 0; i < specialCol_.size(); i++){
    //     cout<<specialCol_[i]<<" ";
    // }
    // cout<<endl;

    if(architecture.size() > 1){
        colWidth = abs(architecture[0][0].x() - architecture[1][0].x());
    }

    fin.close();

    archWidth = architecture.size();

    for(unsigned int i = 0; i < archWidth; i++){
        if(architecture[i].size() > 1 && architecture[i][0].t == "CLB"){
            CLBHeight = architecture[i][1].y() - architecture[i][0].y();
            break;
        }
    }
    for(unsigned int i = 0; i < archWidth; i++){
        if(architecture[i].size() > 1 && architecture[i][0].t == "RAM"){
            RAMHeight = architecture[i][1].y() - architecture[i][0].y();
            break;
        }
    }
    for(unsigned int i = 0; i < archWidth; i++){
        if(architecture[i].size() > 1 && architecture[i][0].t == "DSP"){
            DSPHeight = architecture[i][1].y() - architecture[i][0].y();
            break;
        }
    }
    // cout<<CLBHeight<<" "<<RAMHeight<<" "<<DSPHeight<<endl;
}

void FPGA::Placement::readFile(string archFilePath, string insFilePath, string netFilePath){
    readFileInstance(insFilePath);
    readFileNet(netFilePath);
    readFileArch(archFilePath);

    netNum = net.size();
    instanceNum = instance.size();
    mkInsNet();
}

void FPGA::Placement::outResult(string filePath){
    ofstream fout;
    fout.open(filePath);

    if (!fout.is_open()){
        cerr<<"error: Can't open "<<filePath<<endl;
        exit(1);
    }

    for(unsigned int i = 0; i < instanceNum; i++){
        if(instance[i].InsType == "IO"){
            continue;
        }
        fout<<instance[i].name<<" "<<instance[i].finalResource<<endl;
    }

    fout.close();
}

void FPGA::Placement::badPlace(){

    unsigned int CLB = 0, RAM = 0, DSP = 0, IO = 0;
    for(unsigned int i = 0; i < instanceNum; i++){
        if(instance[i].InsType == "CLB"){
            CLB++;
        }
        else if(instance[i].InsType == "RAM"){
            RAM++;
        }
        else if(instance[i].InsType == "DSP"){
            DSP++;
        }
        else if(instance[i].InsType == "IO"){
            IO++;
        }
    }
    // cout<<"instance: ("<<CLB<<", "<<RAM<<", "<<DSP<<", "<<IO<<")"<<endl;
    
    CLB = 0; RAM = 0; DSP = 0; IO = 0;
    for(unsigned i = 0; i < architecture.size(); i++){
        for(unsigned j = 0; j < architecture[i].size(); j++){
            if(architecture[i][j].t == "CLB"){
                CLB++;
            }
            else if(architecture[i][j].t == "RAM"){
                RAM++;
            }
            else if(architecture[i][j].t == "DSP"){
                DSP++;
            }
            else if(architecture[i][j].t == "IO"){
                IO++;
            }
        }
    }
    // cout<<"arch: ("<<CLB<<", "<<RAM<<", "<<DSP<<", "<<IO<<")"<<endl;

    for(unsigned int i = 0; i < instanceNum; i++){
        if(instance[i].InsType == "IO"){
            continue;
        }
        bool done = false;
        for(unsigned int j = 0; j < architecture.size(); j++){
            for(unsigned int k = 0; k < architecture[j].size(); k++){
                if(architecture[j][k].isMatch == false && architecture[j][k].t == instance[i].InsType){
                    
                    architecture[j][k].isMatch = true;
                    architecture[j][k].matchInsName = instance[i].name;
                    architecture[j][k].matchInsID = i;
                    instance[i].x = architecture[j][k].x();
                    instance[i].y = architecture[j][k].y();
                    instance[i].finalResource = architecture[j][k].name;
                    instance[i].resPos = pair<unsigned int, unsigned int>(j, k);

                    done = true;
                    break;
                }
            }
            if(done == true){
                break;
            }
        }
        if(done == false){
            cerr<<"error: FPGA::Placement::badPlace!"<<endl;
            cerr<<instance[i].name<<" "<<instance[i].InsType<<endl;
            exit(1);
        }
    }

}

bool FPGA::Placement::specialCol(unsigned int i){
    return specialCol_[i];
}

long long FPGA::Placement::getHPWL(){
    double result = 0;
    for(unsigned int i = 0; i < netNum; i++){
        double minX = INT_MAX, maxX = INT_MIN;
        double minY = INT_MAX, maxY = INT_MIN;
        for(unsigned int j = 0; j < net[i].netSize; j++){
            if(instance[net[i].pinIns[j]].x < minX){
                minX = instance[net[i].pinIns[j]].x;
            }
            if(instance[net[i].pinIns[j]].x > maxX){
                maxX = instance[net[i].pinIns[j]].x;
            }
            if(instance[net[i].pinIns[j]].y < minY){
                minY = instance[net[i].pinIns[j]].y;
            }
            if(instance[net[i].pinIns[j]].y > maxY){
                maxY = instance[net[i].pinIns[j]].y;
            }
        }   
        result += abs(maxX - minX) + abs(maxY - minY);
    }

    return result;
}

void FPGA::Placement::mkInsNet(){
    for(unsigned int i = 0; i < netNum; i++){
        for(unsigned int j = 0; j < net[i].netSize; j++){
            unsigned int idx = net[i].pinIns[j];
            instance[idx].netID.push_back(i);
        }
    }
}

unsigned int FPGA::Placement::findXPos(double x){
    int left = 0, right = archWidth-1;
    while(left < right){
        int mid = (left + right)/2;
        if(architecture[mid][0].x() > x){
            right = mid-1;
        }
        else if(architecture[mid][0].x() < x){
            left = mid+1;
        }
        else{
            return mid;
        }
    }
    return right;
}

unsigned int FPGA::Placement::findYPos(unsigned int x, double y){
    int left = 0, right = architecture[x].size()-1;

    while(left < right){
        int mid = (left + right)/2;
        if(architecture[x][mid].y() > y){
            right = mid-1;
        }
        else if(architecture[x][mid].y() < y){
            left = mid+1;
        }
        else{
            return mid;
        }
    }
    return left;
}
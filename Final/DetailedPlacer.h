#pragma once

#include "FPGA.h"

#include <climits>

class NetCost{
public:
    NetCost(){};
    NetCost(double, double, double, double, double);
    ~NetCost(){};

    double cost = 0;
    double Xmax = INT_MIN, Xmin = INT_MAX;
    double Ymax = INT_MIN, Ymin = INT_MAX; 
};

class DetailedPlacer{
public:
    DetailedPlacer(FPGA::Placement &, bool &);
    ~DetailedPlacer(){};

    double insCost(unsigned int insID, double x, double y);
    double insCost(unsigned int insID);
    void getPriority();
    void localMove(unsigned int insID);
    void globalMove(unsigned int insID);
    void place(bool &timeOut);
    void initPlace();
    void updateNetCost(unsigned int);

    void swap(unsigned int insID, unsigned int idxX, unsigned int idxY);
    double trySwap(unsigned int insID, unsigned  idxX, unsigned  idxY);
    vector<pair<unsigned int, double>> priority;

private:
    FPGA::Placement &p;
    bool &timeOut;
    int localMaskSize = 1; // Size: 2*maskSize + 1
    vector<NetCost> netCost;
};
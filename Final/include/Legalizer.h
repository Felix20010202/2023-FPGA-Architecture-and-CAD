#pragma once

#include "FPGA.h"
#include <vector>
using namespace std;

class Cluster{
public:
    Cluster(){};
    ~Cluster(){};
    vector<unsigned int> ins;
};

class Subspace{
public:
    Subspace(){};
    Subspace(unsigned int, unsigned int, int, int);
    ~Subspace(){};
    unsigned int begin = 0, end = 0;
    int beginX = 0, endX = 0;
    vector<unsigned int> neighborRAM;
    vector<unsigned int> neighborDSP;
    vector<pair<unsigned int,pair<double, double>>> ins;
private:

};

class Legalizer{
public:
    Legalizer(): placement_(defaultPlacement){};
    Legalizer(FPGA::Placement &);
    ~Legalizer(){};

    unsigned int MAX_DISTANCE = 10;

    void legal();
    void divSubspace();
    unsigned int spaceNum();
    void getSpacePriority();
    void abacus(unsigned int);
    void tetris();
    void putIns(unsigned int, unsigned int, unsigned int);
    pair<bool, unsigned int> findColPutPos(unsigned int, double);
    void adjust();
    void sortCol(unsigned int);
    void adjustCol(unsigned int);

    vector<Subspace> subspaces;

private:
    FPGA::Placement &placement_;
    static FPGA::Placement defaultPlacement;

    static bool cmp(pair<unsigned int,pair<double, double>> &a, pair<unsigned int,pair<double, double>>&b);

    vector<unsigned int> spacePriority;
    vector<vector<unsigned int>> colSorted;
};
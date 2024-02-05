#include "FPGA.h"
#include "Legalizer.h"
#include "DetailedPlacer.h"

#include<signal.h>
#include<unistd.h>
#include <iostream>
#include <chrono>

using namespace std;

bool timeOut = false;
void sigalrm_handler(int)
{
    timeOut = true;
}

int main(int argc, char **argv){
    if(argc != 5){
        cout<<"Usage:"<<endl<<"./legalizer <first input file path> <second input file path> <third input file path> <output file path>"<<endl;
        exit(1);
    }

    // set process time
    signal(SIGALRM, &sigalrm_handler);
    alarm(590);

    FPGA::Placement placement;
    placement.readFile((string)argv[1], (string)argv[2],(string)argv[3]);
    

    long long GPWL = placement.getHPWL();

    cout<<"========================================"<<endl;
    cout<<"Global placement HPWL: "<<GPWL<<endl;
    cout<<"========================================"<<endl<<endl;
    // Legalizer (For Testing)
    // placement.badPlace();

    auto t1 = chrono::high_resolution_clock::now();
    Legalizer legalizer(placement);
    legalizer.legal();
    auto t2 = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> duration1 = t2 - t1;

    long long LGWL = placement.getHPWL();

    cout<<"========================================"<<endl;
    cout<<"Legalization HPWL: "<<LGWL<<endl;
    cout<<(LGWL - GPWL)*100/(double)GPWL<<" % wirelength"<<endl;
    cout<<"Legalization time: "<<duration1.count()/1000<<"s"<<endl;
    cout<<"========================================"<<endl<<endl;

    auto t3 = chrono::high_resolution_clock::now();
    DetailedPlacer DP(placement, timeOut);
    cout<<"========================================"<<endl;
    DP.place(timeOut);
    auto t4 = chrono::high_resolution_clock::now();
    chrono::duration<double, std::milli> duration2 = t4 - t3;
    long long DPWL = placement.getHPWL();
    cout<<"Detailed placement HPWL: "<<DPWL<<", "<<(DPWL-LGWL)*100/(double)LGWL<<" % "<<endl;
    cout<<"Detailed placement time: "<<duration2.count()/1000<<"s"<<endl;
    cout<<"========================================"<<endl<<endl;
    
    placement.outResult((string)argv[4]);
    return 0;
}
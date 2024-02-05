#pragma once

#include <vector>
#include <string>
#include <unordered_map>

using namespace std;

namespace FPGA{
    class Net{
    public:
        Net(){};
        Net(string n);
        ~Net(){};

        string name;
        unsigned int netSize = 0;
        vector<string> insName;
        vector<unsigned int> pinIns;
    };

    class Instance{
    public:
        Instance(){};
        Instance(string n, string t, double x, double y);
        ~Instance(){};
        string name;
        string InsType;
        string finalResource = "None";
        double x = 0;
        double y = 0;
        vector<unsigned int> netID;
        pair<unsigned int, unsigned int> resPos = pair<unsigned int, unsigned int>(-1,-1);
    };

    class Architecture{
    public:
        Architecture(){};
        Architecture(string, string, double, double);
        ~Architecture(){};   

        double x();
        double y();
        void clear();

        string name;
        string t;

        bool isMatch = false;
        string matchInsName = "None";
        unsigned int matchInsID = -1;
    private:
        double x_, y_;

    };

    class Placement{
    public:
        Placement(){};
        ~Placement(){};


        void readFile(string, string, string);
        void outResult(string);

        void badPlace();

        // accessor
        bool specialCol(unsigned int i);

        long long getHPWL();

        vector<Net> net;
        vector<Instance> instance;
        vector<vector<Architecture>> architecture;

    
        unsigned int instanceNum = 0;
        unsigned int netNum = 0;
        unsigned int archWidth = 0;
        double colWidth = 0;

        double CLBHeight = 0, RAMHeight = 0, DSPHeight = 0;

        unsigned int findXPos(double x);
        unsigned int findYPos(unsigned int x, double y);

    private:
        void readFileInstance(string);
        void readFileNet(string);
        void readFileArch(string);
        void mkInsNet();

        unordered_map<string, unsigned int> insMap;
        unordered_map<string, unsigned int> netMap;

        vector<bool> specialCol_;
    };
}
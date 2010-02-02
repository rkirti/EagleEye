#include "circuit.h"
#include <queue>
#include  <fstream>
using namespace std;


ofstream CIRCUIT_DFILE;
ifstream faultsFile;
extern ofstream EVALUATE_DFILE;
extern ofstream ATPG_DFILE;
extern ofstream MAIN_DFILE;


bool Circuit::ReadFaults()
{
    
    faultsFile.open("tests/faults.txt",ios::in);
    // Read faults from the faults.txt file
    if (!faultsFile.good())
    {
        cout << "Couldn't open the file tests/faults.txt" << endl;
        exit(-1);
    }
    
    string wireName;
    int faultType;
    map<string,Wire *>::iterator iter;

    while (faultsFile >> wireName >> faultType)
    {
        
        cout << "Fault specified: Wire: " << wireName << " FaultValue " << faultType << endl;
        if ( ((iter = Netlist.find(wireName)) !=  Netlist.end()) &&  (faultType == 0 || faultType == 1))
        {
            Fault readFault(iter->second, (faultType==0)?0:1);
            FaultSet.push_back(readFault);
        }
        else
        {
            cout << "Fault specified in tests/faults.txt incorrect - wire not found or faultvalue incorrect" << endl; 
            exit(-1);
        }
    }

    faultsFile.close();
    return true;
}

vector<Value> Circuit::CaptureOutput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriOutputs.begin();
    for (;iter != PriOutputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}



#include "circuit.h"
#include <queue>
#include  <fstream>
using namespace std;


ofstream CIRCUIT_DFILE;
ifstream faultsFile;
extern ofstream EVALUATE_DFILE;
extern ofstream ATPG_DFILE;
extern ofstream MAIN_DFILE;




vector<Value> Circuit::CaptureOutput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriOutputs.begin();
    for (;iter != PriOutputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}


vector<Value> Circuit::CaptureInput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriInputs.begin();
    for (;iter != PriInputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}




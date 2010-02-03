/*
 * Fault and FaultSet definitions
 */
#ifndef FAULT_H
#define FAULT_H

#include "circuit.h"



class Fault{
    public:
    Wire *FaultSite;
    bool faultType;     // s-a-0 or s-a-1
    bool detected;

    //Constructor
    Fault(Wire *faultwire,int type)
    {
        if ((type != 0) && (type != 1))
        {
            cout << "Unknown fault type" << endl;
            assert(false);
        }
        FaultSite = faultwire;
        faultType = type;
        detected=false;
    }

};

typedef list<Fault> FaultSet;

class Circuit;

bool Read_Faults_Into_FaultSet(string fname, Circuit& circuit,FaultSet& set);
void Print_FaultSet(FaultSet set);




#endif /* ifndef FAULT_H */


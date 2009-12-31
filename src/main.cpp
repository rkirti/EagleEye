#include "circuit.h"
#include "atpg.h"
#include "lexer.h"

using namespace std;

/* The global circuit */
Circuit circuit;
extern ofstream ATPG_DFILE;
ofstream MAIN_DFILE;


int main(int argc,char **argv)
{

    ATPG curTest;

    if (argc != 2)
    {
        cout << "Usage: ./bin/atpg <benchmarkfile> " << endl;
        exit(0);
    }


    /*
     * Call the lexer now !
     */

    if( !lexer(argc,argv) )
    {
        cout << ":(" << endl;
        exit(0);
    }

    // Open all the files needed for writing debug info
    circuit.Init_Debug();
    
    // Levels are needed for evaluation
    circuit.Levelize();

    // Name the branch wires correctly    
    circuit.ResolveBranches();

    // Generate random inputs
    curTest.Generate_Random_Vectors();    
    
    // Write both possible faults for each wire in the fault file
    curTest.Generate_Full_FaultSet();
    
    // Read the faults
    circuit.ReadFaults(); 

    // Run ATPG on the fault set
    int detectedFaults=0;
    int undetectedFaults=0;
    list<Fault>::iterator it = circuit.FaultSet.begin();
    for (; it != circuit.FaultSet.end(); it++)
    {
        circuit.Clear_Wire_Values();	
        while (!ImpliQueue.empty())
            ImpliQueue.pop();
        Logs.clear();
    	bool result = curTest.Do_ATPG(it->FaultSite,(it->faultType == 0) ? D : DBAR);
        if (result) 
        {
            MAIN_DFILE  << "Ran D algo SUCCESSFULLY for wire  " << it->FaultSite->id << " for the fault s-a-" << it->faultType << endl;
            MAIN_DFILE << "Outputs at which fault is detected" << endl;

            detectedFaults++;
            // Iterate through list of POs to see if any of them has 
            // D or DBAR. If so, print them   
            map<string,Wire*>::iterator iter = (circuit.PriOutputs).begin();
            for (; iter!=(circuit.PriOutputs).end();iter++)
            {
                if (iter->second->value == D ||  iter->second->value == DBAR )
                    MAIN_DFILE << iter->second->id << "   " <<  iter->second->value
                        << endl;
            }

        }
        else
        {
            MAIN_DFILE  << "Ran D algo but failed for wire  " << it->FaultSite->id << " for the fault s-a-" << it->faultType <<  "  and the result is  " << result << endl;
            MAIN_DFILE << " FAULT NOT DETECTABLE " << endl;
           undetectedFaults++; 
        }   
            MAIN_DFILE << endl << endl;

    }

    MAIN_DFILE << "Total Faults detected :        " << detectedFaults << endl;  
    MAIN_DFILE << "Total Faults not detected :    " << undetectedFaults << endl;  
    return 0;
}


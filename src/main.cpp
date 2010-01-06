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


    if (argc != 2)
    {
        cout << "Usage: ./bin/atpg <benchmarkfile> " << endl;
        exit(0);
    }


    // Call the lexer now !

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

    // Write both possible faults for each wire in the fault file
    Generate_Full_FaultSet();

    // Read the faults
    circuit.ReadFaults(); 

    // RandomVectorTest object to handle random vector tests
    RandomVectorTest rTest;

    // Fault Set generated, written to a file and read into the FaultSet
    // strcuture. Random vectors also available in a file.
    // Now testing.
    rTest.PerformTest(0);  // doesn nothing is no of vectors = 0;

    // ATPG object to handle atpg algorithm
    ATPG atpgTest;

    //    Just leave these lines so that they can used to test individual wires easily - kashyap
    //    map<string,Wire *>::iterator iter = circuit.Netlist.find("h");
    //    bool result = atpgTest.Do_ATPG(iter->second, D);
    //    cout << "the grand result is " << result << endl;

    // Run ATPG on the fault set
    int detectedFaults=0;
    int undetectedFaults=0;
    list<Fault>::iterator it = circuit.FaultSet.begin();
    for (; it != circuit.FaultSet.end(); it++)
    {
        // Important to clean up stuff from the previous run before we begin
        // Set all wires to U
        circuit.Clear_Wire_Values();
        // No Implications or logs should be there.    
        while (!ImpliQueue.empty())
            ImpliQueue.pop();
        Logs.clear();
        // Clear all the Frontiers.
        circuit.DFrontier.clear();
        circuit.JFrontier.clear();


        bool result = atpgTest.Do_ATPG(it->FaultSite,(it->faultType == 0) ? D : DBAR);
        if (result) 
        {
            cout  << "Ran D algo SUCCESSFULLY for wire  " << it->FaultSite->id << " for the fault s-a-" << it->faultType << endl;
            cout << "Outputs at which fault is detected" << endl;

            detectedFaults++;
            // Iterate through list of POs to see if which of them has 
            // D or DBAR.   
            map<string,Wire*>::iterator iter = (circuit.PriOutputs).begin();
            for (; iter!=(circuit.PriOutputs).end();iter++)
            {
                if (iter->second->value == D ||  iter->second->value == DBAR )
                    MAIN_DFILE << iter->second->id << "   " <<  iter->second->value
                        << endl;
            }
            // Open the debug file afresh.
             // We dont need debug info for runs that were
             // successful.
             ATPG_DFILE.close();
             ATPG_DFILE.open("debug/atpg.debug",ios::out);
        }
        else
        {
            cout  << "Ran D algo but failed for wire  " << it->FaultSite->id << " for the fault s-a-" << it->faultType <<  "  and the result is  " << result << endl;
            cout << " FAULT NOT DETECTABLE " << endl;
            undetectedFaults++;
            //            MAIN_DFILE << "DEBUG ME NOW. I am exiting" << endl;
            //            cout << "I am " <<  it->FaultSite->id<< " DEBUG ME NOW. I am exiting" << endl;
            //            exit(0);

        }   
        MAIN_DFILE << endl << endl;

    }

    MAIN_DFILE << "Total Faults detected :        " << detectedFaults << endl;  
    MAIN_DFILE << "Total Faults not detected :    " << undetectedFaults << endl;  
    return 0;
}


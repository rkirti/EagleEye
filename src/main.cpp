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
    Init_Debug();

    // Levels are needed for evaluation
    Levelize(circuit);

    // Name the branch wires correctly    
    Resolve_Branches(circuit);

//    Simulate_Good(circuit);

    // Write both possible faults for each wire in the fault file
    Generate_Full_FaultSet();

    // Read the faults
    circuit.ReadFaults(); 

    // RandomVectorTest object to handle random vector tests
    RandomVectorTest rTest;

    // Fault Set generated, written to a file and read into the FaultSet
    // strcuture. Random vectors also available in a file.
    // Now testing.
    rTest.PerformTest(10,1);  // 10% in 1 min

    // ATPG object to handle atpg algorithm
    ATPG atpgTest;

    //    Just leave these lines so that they can used to test individual wires easily - kashyap
    //    map<string,Wire *>::iterator iter = circuit.Netlist.find("h");
    //    bool result = atpgTest.Do_ATPG(iter->second, D);
    //    cout << "the grand result is " << result << endl;

    // Run ATPG on the fault set
    atpgTest.PerformTest();
    return 0;

}


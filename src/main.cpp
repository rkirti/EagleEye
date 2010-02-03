#include "circuit.h"
#include "fault.h"
#include "atpg.h"
#include "lexer.h"


using namespace std;


/// A global circuit is used for now to facilitate easy interaction with the
//  parsing  module
Circuit circuit;

/// Declaring the debug files.
ofstream MAIN_DFILE;
extern ofstream ATPG_DFILE;


int main(int argc,char **argv)
{

    if (argc != 2)
    {
        cout << "Usage: ./bin/atpg <benchmarkfile> " << endl;
        exit(0);
    }


    /// Call the lexer to parse the verilog description and populate the circuit
    //  data structure.

    if( !lexer(argc,argv) )
    {
        cout << ":(" << endl;
        exit(0);
    }

    /// Open all the files needed for writing debug info.
    Init_Debug();

    /// Levels are needed for evaluation.
    Levelize(circuit);

    /// Name the branch wires correctly.    
    Resolve_Branches(circuit);


    /// Write both possible faults for each wire in the fault file.
    Generate_Full_FaultSet();

    /// Create a test object to run ATPG
    Test curTest;

    /// Read the faults from a pre-specified file into the test object's data structure.
    Read_Faults_Into_FaultSet( "tests/faults.txt",circuit, mytest.faults); 
    Print_FaultSet(mytest.faults);

    /// Finally run ATPG on the fault set.
    mytest.tests =  atpgTest.PerformTest(circuit, mytest.faults);
    mytest.Print_Test_Set();
    return 0;

}





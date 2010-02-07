
/** 
 * @mainpage  EagleEye - [An Automated Test Pattern Generator]
 * @version  
 * 0.01
 * @auth 
 * Kirtika Ruchandani, Kashyap Garimella
 * @date 3/2/2010
 *
 * This program has been developed as a part of the CS633 course at IIT Madras,
 * Digital Systems Testing and Testable Design. The aim is to take a verilog
 * description of a simple combinational circuit (supports all gates including
 * XOR and BUF, which are used in the ISCAS '89 benchmarks) and to output 
 * a set of test vectors which cover all the detectable faults in that circuit.
 *
 *
 * @note
 * 1. This code is under GIT version control.Please use the latest source from
 * http://github.com/rkirti/EagleEye . 
 * Comments,flames and suggestions are welcome either on the github
 * comments section or at kirtibr@gmail.com.
 * 2. You need to have a recent version of lex and yacc installed 
 * to be able to compile.
 */


#include "circuit.h"
#include "fault.h"
#include "atpg.h"
#include "lexer.h"


using namespace std;





/// A global circuit is used for now to facilitate easy interaction with the
///  parsing  module
Circuit circuit;

/// Declaring the debug files.
ofstream MAIN_DFILE;
extern ofstream ATPG_DFILE;


/**
 * @brief
 *
 * Main Program - Takes care of the complete flow from calling the lexer to 
 * parse the verilog description, setting up the circuit structures to 
 * setting up the fault sets for the test and actually performing the
 * test and capturing the test vectors. Please check the main.debug
 * in the debug folder after running the code to get a concise picture of
 * the whole run.
 *
 * @param The main function takes in the name of the verilog file which has the circuit
 * description.
 *
 * @return Always returns a value of 0.
 */

int main(int argc,char **argv)
{
    string ofile;
        
    if (argc != 3)
    {
        cout << "Usage: ./bin/atpg <benchmarkfile> <output stats file>" << endl;
        exit(0);
    }

    ofile = argv[2];
    /// MAIN FUNCTION TASKS

    ///  Call the lexer to parse the verilog description and populate the circuit
    ///  data structure.

    if( !lexer(argc,argv) )
    {
        cout << ":(" << endl;
        exit(0);
    }

    /// Open all the files needed for writing debug info.
    Init_Debug(ofile);

    /// Levels are needed for evaluation.
    Levelize(circuit);

    /// Name the branch wires correctly.    
    Resolve_Branches(circuit);


    /// Write both possible faults for each wire in the fault file.
    Generate_Full_FaultSet();

    /// Create a test object to run ATPG
    ATPG curTest;

    /// Read the faults from a pre-specified file into the test object's data structure.
    Read_Faults_Into_FaultSet( "tests/faults.txt",circuit, curTest.faults); 
    Print_FaultSet(curTest.faults);

    /// Finally run ATPG on the fault set.
    curTest.tests =  curTest.PerformTest(circuit, curTest.faults);
    curTest.Print_Test_Set();
    return 0;

}





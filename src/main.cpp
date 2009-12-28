#include "circuit.h"
#include "atpg.h"
#include "lexer.h"

using namespace std;

/* The global circuit */
Circuit circuit;
extern ofstream ATPG_DFILE;



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

    circuit.Init_Debug();

	circuit.Levelize();
	circuit.ResolveBranches();
	

    circuit.Print_All_Wires();
    
    
    
    //Try to run ATPG for each wire in the ckt
    curTest.Do_ATPG("N16",D);	





     // circuit.Simulate_Good();
    // cout << "finished evaluating" << endl;

	/*
	 * print the circuit, just to test it
	 */

/*	
    map<string,Wire*>::iterator iter=(circuit.Netlist).begin();
	
	for(;iter != (circuit.Netlist).end(); iter++)
	{
		cout << iter->first << "  " << iter->second->wtype  << endl;
                list<Element*>::iterator iterEle = iter->second->outputs.begin();
                for(;iterEle != iter->second->outputs.end(); iterEle++)
                    cout << (*iterEle)->id << "  " ;
                cout << endl;
                //cout << iter->second->input->id << "  = input " << endl;
	}
    map<string,Gate*>::iterator iter1=(circuit.Gates).begin();
	
	for(;iter1 != (circuit.Gates).end(); iter1++)
	{
		cout << iter1->first << "  " << iter1->second->gtype << " level="<< iter1->second->level << "  " << endl;
	}
*/	
    
    
    
    
    return 0;
}



		


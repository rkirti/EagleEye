#include "circuit.h"
#include "atpg.h"
#include "lexer.h"

using namespace std;

/* The global circuit */
Circuit circuit;


int main(int argc,char **argv)
{

    ATPG curTest;
	/*
	 * Call the lexer now !
	 */
	if( !lexer(argc,argv) )
	{
		cout << ":(" << endl;
		exit(0);
	}

	circuit.Levelize();
	circuit.ResolveBranches();
	
    curTest.Do_ATPG();	

    //   circuit.Evaluate();
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



		


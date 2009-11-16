#include "circuit.h"
#include "atpg.h"
#include "lexer.h"

using namespace std;

/* The global circuit */
Circuit circuit;


int main(int argc,char **argv)
{

	/*
	 * Call the lexer now !
	 */
	if( !lexer(argc,argv) )
	{
		cout << ":(" << endl;
		exit(0);
	}

	circuit.Levelize();
	//circuit.ResolveBranches();
	//Do_ATPG();	// :P

        circuit.Evaluate();

	/*
	 * print the circuit, just to test it
	 */
	/*
    map<string,Wire*>::iterator iter=(circuit.Netlist).begin();
	
	for(;iter != (circuit.Netlist).end(); iter++)
	{
		cout << iter->first << "  " << iter->second->wtype << "  " << iter->second->value << endl;
	}
	
    map<string,Gate*>::iterator iter1=(circuit.Gates).begin();
	
	for(;iter1 != (circuit.Gates).end(); iter1++)
	{
		cout << iter1->first << "  " << iter1->second->gtype << " level="<< iter1->second->level << "  " << endl;
	}
	
    */
    
    
    
    
    return 0;
}



		


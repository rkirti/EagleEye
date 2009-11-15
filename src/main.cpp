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
	
	Do_ATPG();	// :P

	/*
	 * print the circuit, just to test it
	 */
	
	map<string,Wire*>::iterator iter=(circuit.Netlist).begin();
	
	for(;iter != (circuit.Netlist).end(); iter++)
	{
		cout << iter->first << iter->second->wtype << "  " << iter->second->value << endl;
	}



	return 0;
}



		


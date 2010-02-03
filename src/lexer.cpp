/**
 * @file
 *  
 * This file defines the wrapper function to be used by the lexer,
 * to populate the wire and gate elements of the circuit while parsing the 
 * verilog file.
 * Lex and yacc do not integrate well with c++ code, 
 * so we use this hack and make only a c compatible portion
 * visible.
 */


#include "circuit.h"
#include "lexer.h"

using namespace std;


/**
 *
 *
 */

int Lexer_AddWire(char* inName,WireType type)
{   /// Call the actual cpp function for adding a wire
	return Add_Wire(circuit,inName,type);

}

int Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
    /// Call the actual cpp function for adding a gate
	return Add_Gate(circuit,type, name, output, inputs, numSignals);
}

/**
 * @file
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
 * @param inName
 * This is the wire name passed on by the lexer.
 *
 * @param type
 * The wire type parameter specifies whether it is 
 * a Primary Input (PI) , Primary Output (PO)
 * or a connection.
 * Check the enum WireType in enumdefs.h 
 *
 * @return
 * Returns 1 (true) if the wire was successfully added
 * to the Netlist of the circuit. Else it returns 0.
 *
 * @see: Check the Add_Wire function which does the actual
 * wrapping to see possible reasons for failure to add a wire name
 * A listing of the sanity checks which are implemented is provided
 * there.
 */

int Lexer_AddWire(char* inName,WireType type)
{   /// Call the actual cpp function for adding a wire
	return Add_Wire(circuit,inName,type);

}


/**
 *
 * @param type
 * The gate type specifies what gate  this is-
 * supported gate types include AND,OR,NOT,NAND,
 * NOR, XOR and BUF.
 * Check the enum GateType in enumdefs.h 
 *
 * @param name
 * Gate name passed on by the lexer.
 * 
 * @param output
 * Name of the output wire of the gate
 * passed on by the lexer.
 *
 * @param inputs
 * A character array listing the names of all input wires.
 * 
 * @param numSignals
 * Number of inputs of the gate.
 *
 * @return
 * Returns 1 (true) if the wire was successfully added
 * to the Netlist of the circuit. Else it returns 0.
 * @see: Check the Add_Gate function in setup.cpp which does the actual
 * wrapping to see possible reasons for failure to add a gate name
 * A listing of the sanity checks which are implemented is provided
 * there.
 */

int Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
    /// Call the actual cpp function for adding a gate
	return Add_Gate(circuit,type, name, output, inputs, numSignals);
}

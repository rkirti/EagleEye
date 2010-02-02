#include "circuit.h"
#include "lexer.h"

using namespace std;

int Lexer_AddWire(char* inName,WireType type)
{
	return Add_Wire(circuit,inName,type);

}

int Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
	return Add_Gate(circuit,type, name, output, inputs, numSignals);
}

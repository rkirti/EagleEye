#include "circuit.h"
#include "lexer.h"

using namespace std;

bool Lexer_AddWire(char* inName,WireType type)
{
	return circuit.AddWire(inName,type);

}

bool Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
	return circuit.AddGate(type, name, output, inputs, numSignals);
}
	


bool lexer()	// dummy lexer, written by GJ
{
	char n1[]="N1", n2[]="N2", n3[]="N3";
	char* n12[3] = { (char *)"N1",(char *)"N2" };
	Lexer_AddWire(n1,PI);
	Lexer_AddWire(n2,PI);
	Lexer_AddWire(n3,PO);

	Lexer_AddGate(AND,(char *)"AND2_0",n3,n12,2);

	return true;
}

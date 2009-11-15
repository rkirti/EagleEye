
/*
 * LEXER Related functions for class Circuit
 */
#ifndef LEXER_H
#define LEXER_H


/*
 * Define global functions to be used by lexer 
 * to add elements to the circuit
 */
extern "C" int Lexer_AddWire(char *inName,WireType type);
extern "C" int Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals);
extern "C" int lexer(int argc,char **argv);	// should be renamed to the yacc lexer function

#endif /* ifndef LEXER_H */


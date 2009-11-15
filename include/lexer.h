
/*
 * LEXER Related functions for class Circuit
 */
#ifndef LEXER_H
#define LEXER_H

#include "circuit.h"

/*
 * Define globla functions to be used by lexer 
 * to add elements to the circuit
 */
extern bool Lexer_AddWire(char *inName,int type);
extern bool Lexer_AddGate(GateType type, char *name,char* output,char **inputs,int numSignals);

bool lexer(void);	// should be renamed to the yacc lexer function

#endif /* ifndef LEXER_H */


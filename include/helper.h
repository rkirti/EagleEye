
/*
 * Generic Helper Functions
 */
#ifndef HELPER_H
#define HELPER_H

#include "circuit.h"
#include <string>


bool isNotKnown(Value);
string intToString(int inInt);
Value Do_Xor(Value val1, Value val2);
int Translate_Value_To_Int(Value value);
void Init_Debug();
void Print_All_Wires(Circuit& circuit);
void Clear_Wire_Values(Circuit& circuit);
void Clear_Internal_Wire_Values(Circuit& circuit);
    


#endif /* ifndef HELPER_H */

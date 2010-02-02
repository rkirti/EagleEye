
/*
 * Setup class to set up the circuit
 */
#ifndef SETUP_H
#define SETUP_H



bool  Add_Wire(Circuit& circuit,const char *inName,WireType type);
bool Add_Gate(Circuit& circuit,GateType type, char *name,char* output,char **inputs,int numSignals);
void Add_Gate_To_Wire_Output(Circuit& circuit,Gate* gate,const char* wirename);   
void  Add_Gate_To_Wire_Input(Circuit& circuit,Gate* gate,const char* wirename);
bool Levelize(Circuit& circuit);   
bool Resolve_Branches(Circuit& circuit);
void Resolve_Wire(Circuit& circuit,Wire* wire);
bool Wire_Not_Derived(Wire* wire);
string Check_Name_Present(Circuit& circuit,string givenname);
string intToString(int inInt);

#endif /* ifndef SETUP_H */

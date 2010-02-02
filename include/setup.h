
/*
 * Setup class to set up the circuit
 */
#ifndef SETUP_H
#define SETUP_H



bool  AddWire(Circuit& circuit,const char *inName,WireType type);
bool Levelize(Circuit& circuit);   
bool Resolve_Branches(Circuit& circuit);
void Resolve_Wire(Circuit& circuit,Wire* wire);
bool Wire_Not_Derived(Wire* wire);
string Check_Name_Present(Circuit& circuit,string givenname);
string intToString(int inInt);

#endif /* ifndef SETUP_H */

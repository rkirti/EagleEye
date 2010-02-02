
/*
 * Setup class to set up the circuit
 */
#ifndef SETUP_H
#define SETUP_H

    bool Levelize(Circuit* circuit);   
    bool Resolve_Branches(Circuit* circuit);
    void Resolve_Wire(Circuit* circuit,Wire* wire);

    bool Wire_Not_Derived(Wire* wire);

string Check_Name_Present(Circuit* circuitPtr,string givenname);


#endif /* ifndef SETUP_H */
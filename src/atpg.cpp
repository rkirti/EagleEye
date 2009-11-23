#include "circuit.h"

bool Do_ATPG()
{
   Value newVal = D;
   bool result;
    // n10 - forward D 
   Implication* newImply= new Implication(((circuit.Netlist).find("N10"))->second,newVal,true); 
    (circuit.ImpliQueue).push(newImply); 
    // n0 - backward 0
   newImply= new Implication(((circuit.Netlist).find("N0"))->second,ZERO,false); 
    (circuit.ImpliQueue).push(newImply); 
    // n1 - backward 0
   newImply= new Implication(((circuit.Netlist).find("N1"))->second,ZERO,false); 
    (circuit.ImpliQueue).push(newImply); 
//bool false = 0 = backward
   cout << "Adding implication :  " << "N10" 
    << "value:  " <<  newVal;
    (circuit.ImpliQueue).push(newImply); 
   result = circuit.D_Algo();
   cout << "Returning " << result << endl;
}

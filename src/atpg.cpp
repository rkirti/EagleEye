#include "circuit.h"

bool Do_ATPG()
{
   Value newVal = D;
   bool result;
   Implication* newImply= new Implication(((circuit.Netlist).find("N10"))->second,newVal,true); 
//bool false = 0 = backward
   cout << "Adding implication :  " << "N10" 
    << "value:  " <<  newVal;
    (circuit.ImpliQueue).push(newImply); 
    newImply= new Implication(((circuit.Netlist).find("N10"))->second,ONE,false); 
    (circuit.ImpliQueue).push(newImply); 
   result = circuit.D_Algo();
   cout << "Returning " << result << endl;
}

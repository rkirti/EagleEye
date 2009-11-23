#include "circuit.h"

bool Do_ATPG()
{
   Value newVal = D;
   bool result;
   (circuit.faultWire) = ((circuit.Netlist).find("N10")->second);

    // n10 - backward ONE 
   Implication* newImply= new Implication(circuit.faultWire,ONE,false); 
   (circuit.ImpliQueue).push(newImply); 

    // n10 - forward D 
   newImply= new Implication(circuit.faultWire,D,true); 
   (circuit.ImpliQueue).push(newImply); 

//    // n0 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N0"))->second,ZERO,false); 
//    (circuit.ImpliQueue).push(newImply); 
//    // n1 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N1"))->second,ZERO,false); 
//    (circuit.ImpliQueue).push(newImply); 
////bool false = 0 = backward


   cout << "Adding implication :  " << "N10" 
    << "value:  " <<  newVal;
   result = circuit.D_Algo();
   cout << "Returning " << result << endl;
}

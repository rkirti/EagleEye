#include "circuit.h"
#include "atpg.h"

bool ATPG::Do_ATPG()
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
   result = D_Algo();
   
   
   
   Update_PI_For_9V();
   
   cout << "Returning " << result << endl;
   map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    cout << "Test vectors are : " << endl;
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        cout << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

   iter =  (circuit.PriOutputs).begin();
    cout << "Output vectors are : " << endl;
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        cout << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

   iter =  (circuit.Netlist).begin();
    cout << "Netlist : " << endl;
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        cout << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }
}



bool ATPG::D_Algo()
{ 
     //Ensure all implications are made and they 
     // are consistent
    if (circuit.Imply_And_Check() == false) 
    {

        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }
     //Iterate through list of POs to see if any of them has 
     //D or DBAR. If so, return true.   
    
    list<WireValuePair>::iterator checkIter = (circuit.DFrontier).begin();
    map<string,Wire*>::iterator iter = (circuit.PriOutputs).begin();
    
    for (; iter!=(circuit.PriOutputs).end();iter++)
    {
        // D_Algo just takes care of returning true.
        // The input vector should then be recorded 
        // by the Do_ATPG function
        if (iter->second->value == D ||  iter->second->value == DBAR )
            goto JFrontierWork;
    }

DFrontierWork:
    // Error is not at PO. Algo should execute :(
    
    // No means of propagating the error ahead
    if (circuit.DFrontier.empty()) 
    {

        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }
    for (;checkIter !=(circuit.DFrontier).end();checkIter++)
    {
        /*Selecting a gate from the DFrontier*/
       Gate* curGate=dynamic_cast<Gate*>(checkIter->iwire->input);
       Value cval = ControlValues[curGate->gtype];
        
      cout << "Selected gate: " << curGate->id 
           << "from the D Frontier" << endl;
      cout << "Control value is: " << cval << endl;

      list<Wire*>::iterator inputIter = (curGate->inputs).begin();
      for (;inputIter != (curGate->inputs).end();inputIter++)
      {
          if ((*inputIter)->value == U)
            {
                // All implications backward
                
                Implication* newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                cout << __LINE__  << ":  " ;
                cout << "Adding backward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (circuit.ImpliQueue).push(newImply);
                newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                cout << __LINE__  << ":  " ;
                cout << "Adding forward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (circuit.ImpliQueue).push(newImply);
            }         
          else
              cout <<  "DEBUG: Wire: " << (*inputIter)->id << "value: " <<  (*inputIter)->value <<endl; 


      }

     if (D_Algo() == true) return true;
     // Temporary resort
     else assert(false);
    
    }

JFrontierWork:
    if (circuit.JFrontier.empty()) return true;
    for (;checkIter !=(circuit.JFrontier).end();checkIter++)
    {
        /*Selecting a gate from the DFrontier*/
       Gate* curGate=dynamic_cast<Gate*>(checkIter->iwire->input);
       Value cval = ControlValues[curGate->gtype];
        
      cout << __LINE__ ;
      cout << "Selected gate: " << curGate->id 
           << "from the D Frontier" << endl;
      cout << "Control value is: " << cval << endl;
      cout << endl <<  endl;

      list<Wire*>::iterator inputIter = (curGate->inputs).begin();
      for (;inputIter != (curGate->inputs).end();inputIter++)
      {
          if ((*inputIter)->value == U)
            {
                // All implications backward
                Implication* newImply= new Implication(*inputIter,(Value)(cval),false); /*bool false = 0 = backward*/
                cout << "Adding implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
              
                (circuit.ImpliQueue).push(newImply);
                break;
            }         
      }
    
    if (D_Algo() == true) return true;
     // Temporary resort
     else assert(false);
    }
    return false;
}

void ATPG::Update_PI_For_9V()
{
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        if ( ( (iter->second)->value == ZEROBYU )  ||  ( (iter->second)->value == ONEBYU ) 
                || ( (iter->second)->value == UBYONE )  ||  ( (iter->second)->value == UBYZERO ) )
            (iter->second)->value  = U;
    }
}

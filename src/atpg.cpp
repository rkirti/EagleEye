#include "circuit.h"
#include <stdio.h>
#include "atpg.h"
#include "macros.h"
#include "dot.h"

static list<Implication*> Logs;
static queue<Implication*> ImpliQueue;


ofstream ATPG_DFILE;



bool ATPG::Do_ATPG(string name, Value faultval)
{
    bool result;
    string faultWireName;
    Implication* newImply;

    ATPG_DFILE << "ATPG called for faulty wire " <<  name << endl;
    ATPG_DFILE << "Faulty value of the wire is " <<  faultval << endl;
   
    map<string,Wire*>::iterator it = circuit.Netlist.find(name);
    
    if ( it == circuit.Netlist.end())
    {

        ATPG_DFILE << "Given wire " <<  name << " not found " << endl;
        cout << "Wire not found" << endl; 
        exit(0);
    }
    
    (circuit.faultWire) = (it->second); 
    


    // Push two implications for the faulty wire.
    // One backward to justify vbar on the line stuck
    // at v. One forward to propagate the stuck at v error.
    newImply= new Implication(circuit.faultWire,(faultval == D)?ONE:ZERO ,false); 
    (ImpliQueue).push(newImply); 
    newImply= new Implication(circuit.faultWire,faultval,true); 
    (ImpliQueue).push(newImply); 

    ATPG_DFILE << "Adding implication: " <<  (circuit.faultWire)->id  
        << "  value:  " <<  faultval <<  endl;
 
    ATPG_DFILE << "Calling D_Algo from DO_ATPG" << endl;


    // Run the D Algorithm to find the test vectors.
    result = D_Algo();

    // Useless for now, as only D is implemented
    //Update_PI_For_9V();


    // Display results
    ATPG_DFILE << "Returning from D Algo for wire "  <<  name << " with result " << result << endl;
    circuit.Print_All_Wires();
    CircuitGraphPrint();
    
   return result;

}



bool ATPG::Handle_Output_Coming_From_Noncontrol_Value(Implication* curImplication, Wire* curWire,Value impliedValue)
{
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    Value xorValue =  Do_Xor((Value)((~ControlValues[curGate->gtype])&0xf),(Value)InversionValues[curGate->gtype]);
    
 
    ATPG_DFILE << "Handling implication on wire " << curWire->id
          << " non-controlling value on gate inputs needed" << endl; 


    // This function is reached only if the wire on 
    // which the implication is made has a gate 
    // input. i.e it is not a fanout branch and 
    // its value is cbar xor i
    
    ATPG_DFILE << "Value to be implied" << impliedValue << endl;
    ATPG_DFILE << "Value by Xor" <<  xorValue << endl;
    ATPG_DFILE << "Current value of the wire is " << curWire->value << endl;
    ATPG_DFILE << "Current gate's output evaluated is " << curGate->Evaluate();

    assert(impliedValue ==  xorValue);
    assert(curGate);


    // Note the order of actions from here on is
    // important. 


    // ACTION 1:
    // The current value of the wire should be 
    // compatible with the value to be implied
    // else we need backtracking

    if (!Compatible(curWire->value,impliedValue))
    {
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }


    // ACTION 2:
    // If the current value of the wire equals the 
    // implied value, the implication is popped here.
    // This does not mean the task is over, since
    // justifying the inputs for this wire may be
    // pending, which is handled just below this code
    // fragment.

    if (curWire->value == impliedValue) 
    {
        ATPG_DFILE << "Implied value equals the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Popping off the implication" << endl;
        ATPG_DFILE << endl;
        ImpliQueue.pop();
    } 
   
   
    // ACTION 3:
    // The current gate will evaluate to the
    // implied value only if all input values
    // are cbar. Thus, no need to go backward.
    // Return true.
    
    // The second check is to ensure that 
    // we are still talking of the same implication
    // and not popping off something else, since
    // we might have popped in action 2
    if ((curGate->Evaluate() == impliedValue) && (curWire->id == ImpliQueue.front()->wire->id) )
    {
        ATPG_DFILE << "Implied value equals the value of the gate output." << endl;
        ATPG_DFILE << "Gate in question is :" << curGate->id << endl;
        ATPG_DFILE << "Popping off the implication" << endl;
        ATPG_DFILE << endl;

        // Pop off the implication anyways
        // Set the value only if its not
        // the faulty wire
        if ( curWire != (circuit.faultWire) )
            Change_Value_And_Update_Log(curImplication);
        ImpliQueue.pop();
        
        return true;
    } 



    // ACTION 4: 
    // At this point, the gate can evaluate to either 
    // unknown or to c xor i, the latter being the case
    // where backtracking is needed.Check which of these
    // cases applies.

    switch (curGate->Evaluate())
    {   
        case U:
            {
                list<Wire*>::iterator iter = (curGate->inputs).begin();
                for (;iter != (curGate->inputs).end();iter++)
                {
                    // All inputs should be cbar
                    Implication* newImply;
                    Value newVal = (Value)((~(ControlValues[curGate->gtype])) & 0xf);
                    newImply= new Implication(*iter,newVal,false); 
                    ATPG_DFILE << " Adding implication :  " << (*iter)->id << " Value: " <<  newVal << endl;
                    ImpliQueue.push(newImply); 
                }

                // If it is the faulty wire, don't set the value
                if ( curWire != (circuit.faultWire) )
                {
                    ATPG_DFILE << " New intention:  " <<  curImplication->wire->id << "  " <<  curImplication->value << endl; 
                    Change_Value_And_Update_Log(curImplication);
                }
                ImpliQueue.pop();
                return true;

            }


        default:
            // Some input has value c
            {
                ATPG_DFILE << "Evaluate of " << curGate->id << " gives :" << curGate->Evaluate() << endl;
                ATPG_DFILE << "Evaluation not compatible with implied value: " << impliedValue << endl;
                ATPG_DFILE << "Returning false" << endl;
                ATPG_DFILE <<  endl;
                return false;
            }
    }

}



bool ATPG::Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value impliedValue)
{

    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    Value xorValue =  Do_Xor((Value)((ControlValues[curGate->gtype])&0xf),(Value)InversionValues[curGate->gtype]);
    
 
    ATPG_DFILE << "Handling implication on wire " << curWire->id
               << " controlling value on gate inputs needed" << endl; 


    ATPG_DFILE << "Value to be implied" << impliedValue << endl;
    ATPG_DFILE << "Value by Xor" <<  xorValue << endl;
    ATPG_DFILE << "Current value of the wire is " << curWire->value << endl;
    ATPG_DFILE << "Current gate's output evaluated is " << curGate->Evaluate();


    // Assumption: 
    // The implication is on a wire
    // that isn't a PI or a fanout branch
    // They are taken care of separately and 
    // not in this function
    
    int inputUCount = 0;
    list<Wire*>::iterator iter;
    
    assert(impliedValue == xorValue);

    // Note the order of actions from here on is
    // important. 


    // ACTION 1:
    // The current value of the wire should be 
    // compatible with the value to be implied
    // else we need backtracking

    if (!Compatible(curWire->value,impliedValue))
    {
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }

    // ACTION 2:
    // If the current value of the wire equals the 
    // implied value, the implication is popped here.
    // This does not mean the task is over, since
    // justifying the inputs for this wire may be
    // pending, which is handled just below this code
    // fragment.
    if (curWire->value == impliedValue) 
    {
        ATPG_DFILE << "Implied value equals the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Popping off the implication" << endl;
        ATPG_DFILE << endl;
        ImpliQueue.pop();
    } 


    // ACTION 3:
    // The current gate will evaluate to the
    // implied value only if some input value
    // are c. Thus, no need to go backward.
    // Return true.
    
    if ((curGate->Evaluate() == impliedValue) && (curWire->id == ImpliQueue.front()->wire->id) )
    {
        // if it is the faulty wire, don't set the value
        if (curWire != (circuit.faultWire))
            Change_Value_And_Update_Log(curImplication);
        ImpliQueue.pop();
        return true;
    } 

    // ACTION 4:
    // If we came here, no input has value
    // C. Therefore, the following code sets 
    // some input to C.
    // Count the number of unknown inputs. 
    // Further action depends on the count.
    // 0  : Something wrong. We need atleast
    // one U to set it to c.
    // 1  :  Imply on that being C since
    // thats the only way out.
    // >1 : Add the gate to J frontier.
    
    
    iter = (curGate->inputs).begin();
    for (;iter != (curGate->inputs).end();iter++)
    {
        if  ( (*iter)->value  == U) inputUCount++;
    }

    ATPG_DFILE << "Input UCount is " << inputUCount << " for gate " << curGate->id << endl;

    if (inputUCount>1) 
    {
        // if it is the faulty wire, don't set the value
        
        if ( curWire != (circuit.faultWire) )
            Change_Value_And_Update_Log(curImplication);
        
        cout<< "Adding to J Frontier:  " << (*iter)->id << endl;
        Add_To_JFrontier((*iter),impliedValue);
        ImpliQueue.pop();
        return true;
    }

    if (inputUCount == 1)
    {

        // Iterate again to find out which wire
        // has unknown value and imply C on it.
        ATPG_DFILE << "Hunting for the only input wire with value U" << endl; 

        for (iter = (curGate->inputs).begin();iter != (curGate->inputs).end();iter++)
        {

            if (Compatible((*iter)->value,U))
            {

                Value newVal = (ControlValues[curGate->gtype]);
                // false indicates backward direction
                Implication* newImply= new Implication(*iter,newVal,false);
                ATPG_DFILE << (*iter)->id << "is the sole input wire with Value U" << endl;  
                ATPG_DFILE << "Adding implication on that wire with  value:  " <<  newVal << endl;
                ImpliQueue.push(newImply); 

            }
        }

        ImpliQueue.pop();
        
        
        // If it is the faulty wire, don't set the value
        if ( curWire != (circuit.faultWire) )
        {     
            cout<<  curImplication->wire->id << "  " <<  curImplication->value << endl; 
            Change_Value_And_Update_Log(curImplication);
        }    
        return true;
    }

}



bool ATPG::Compatible(Value oldval,Value newval)
{
    if (oldval == newval) return true;

    switch (oldval)
    {
        case U:
            return true;
        case UBYONE:
            if (newval == UBYONE || newval == DBAR || newval == ONE)
            return true;
            else 
            return false;    

        case UBYZERO:
            if (newval == UBYZERO || newval == D || newval == ZERO)
            return true;
            else 
            return false;    

      case ZEROBYU:
            if (newval == ZERO || newval == DBAR || newval == ZEROBYU)
            return true;
            else 
            return false;    


      case ONEBYU:
            if (newval == ONE || newval == D || newval == ONEBYU)
            return true;
            else 
            return false;    
    }
    return false;
}



bool ATPG::Add_To_JFrontier(Wire *wire,Value value)
{
    circuit.JFrontier.push_back(WireValuePair(wire,value));
    ATPG_DFILE  << "Added a gate with output wire " <<  wire->id << " to JFrontier." << endl;
    ATPG_DFILE <<  "JFrontier is now  " << endl; 
    PRINTJFRONTIER;
    ATPG_DFILE << endl << endl;
    return true;
}


bool ATPG::Add_To_DFrontier(Wire *wire,Value value)
{
    circuit.DFrontier.push_front(WireValuePair(wire,value));
    ATPG_DFILE  << "Added a gate with output wire " <<  wire->id << " to DFrontier." << endl;
    ATPG_DFILE <<  "DFrontier is now  " << endl; 
    PRINTDFRONTIER;
    ATPG_DFILE << endl << endl;
    return true;
}



bool ATPG::Remove_From_D(Wire *wire)
{
    bool result=false;
    list<WireValuePair>::iterator iter = circuit.DFrontier.begin();

    while (iter != circuit.DFrontier.end())
    {
        if ( (iter->iwire) == wire )
        {
            iter = circuit.DFrontier.erase(iter);
            result = true;
            continue;   // we can return from function
        }
        iter ++;
    }
    ATPG_DFILE  << "Removed a gate with output wire " <<  wire->id << " from DFrontier." << endl;
    ATPG_DFILE <<  "DFrontier is now  " << endl; 
    PRINTDFRONTIER;
    ATPG_DFILE << endl << endl;
    return result;
}



bool ATPG::Remove_From_J(Wire *wire)
{
    bool result=false;
    list<WireValuePair>::iterator iter = circuit.JFrontier.begin();

    while (iter != circuit.JFrontier.end())
    {
        if ( (iter->iwire) == wire )
        {
            iter = circuit.JFrontier.erase(iter);
            result = true;
            continue;   // we can return from function
        }
        iter ++;
    }
    ATPG_DFILE  << "Removed a gate with output wire " <<  wire->id << " from JFrontier." << endl;
    ATPG_DFILE <<  "JFrontier is now  " << endl; 
    PRINTJFRONTIER;
    ATPG_DFILE << endl << endl;
    return result;
}



bool ATPG::Imply_And_Check()
{
    ATPG_DFILE << "Imply_And_Check called" << endl;
    assert(Logs.empty());
    
    while (!ImpliQueue.empty())
    {

        //  Step 1: Get Details of current implication 
        Implication*  curImplication = (ImpliQueue).front();
        Wire* curWire = curImplication->wire;
        Value curValue = curImplication->value;
        ATPG_DFILE <<  "New implication from the queue:  " << (curWire->id) <<  " value:   "<< (curValue) ;
        if (curImplication->direction == 0)  ATPG_DFILE <<   "  backwards" << endl;
        else  ATPG_DFILE << "  forward" << endl;
            

        //  Resolving backward implication
        if (curImplication->direction == false) 
        {
            if (Resolve_Backward_Implication(curImplication,curWire,curValue) == false)
            {

                ATPG_DFILE << "Backward implication not resolved " ;
                ATPG_DFILE << ": Returning false" << endl << endl;
                goto fail;
            }
                else continue;

        }


        //  Resolving forward implication
        else
        {
            if (Resolve_Forward_Implication(curImplication,curWire,curValue) == false)
         {

             ATPG_DFILE << "Forward implication not resolved " ;
             ATPG_DFILE << ": Returning false" << endl << endl;
             goto fail;

         }
                else continue;

        }
    }

    //Make_Assignments(); 	// Deprecated
    Clean_Logs(); 	// Clears the log
    return true;

fail:
    Failure();
    return false;
}


/*
void ATPG::Make_Assignments()
{
   assert(ImpliQueue.empty());
   list<Implication*>::iterator iter= Logs.begin();
   for (; iter!=Logs.end();iter++)
    {
        // Intended value is on the RHS
        (*iter)->wire->value = (*iter)->value;
    }
   Logs.clear();
}
*/



void ATPG::Clean_Logs()
{
    assert(ImpliQueue.empty());
    list<Implication*>::iterator iter= Logs.begin();
    for (; iter!=Logs.end();iter++)
    {
	    // Old value is on the RHS
	    (*iter)->wire->modified = false;
    }
    Logs.clear();
    return;
}


void ATPG::Failure()
{
    ATPG_DFILE << "Imply_And_Check failed. NEED TO BACKTRACK" << endl << endl;
    ATPG_DFILE << "Using the logs to reverse all decisions/assignments" << endl;
    list<Implication*>::iterator iter= Logs.begin();
    for (; iter!=Logs.end();iter++)
    {
        ATPG_DFILE << "Reverting value of wire" << endl; 
	    // Old value is on the RHS
	    (*iter)->wire->value = (*iter)->value;
	    // reset the modified flag anyways
	    (*iter)->wire->modified = false;
    }
    Logs.clear();
    // remove all the implications thought of
    while (!ImpliQueue.empty())
	    ImpliQueue.pop();
}


bool ATPG::D_Algo()
{ 
     // Ensure all implications are made and they 
     // are consistent
    if (Imply_And_Check() == false) 
    {

        ATPG_DFILE << " Imply_And_Check returned false. Returning false from D_Algo " << endl;
        return false;
    }
   

    // The declarations needed later are made here,
    // even though we are not sure they will be 
    // needed because the compiler
    // doesn't like declarations in the middle of goto
    // jumps
    WireValuePair DFront = (circuit.DFrontier).front();
    WireValuePair *checkIter = &DFront;
    list<WireValuePair>::iterator checkIterator;
    map<string,Wire*>::iterator iter = (circuit.PriOutputs).begin();
  

    // Iterate through list of POs to see if any of them has 
    // D or DBAR. If so, return true.   
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

        ATPG_DFILE  << "Found the D Frontier empty and hence cant propagate fault." << endl;
        ATPG_DFILE  <<  "Returning false" << endl << endl ;
        return false;
    }

    PRINTDFRONTIER;
    
    
    // for (;checkIter !=(circuit.DFrontier).end();checkIter++)
    // Remember a stack and understand how this works
    // Add_to_D_frontier function always pushes into 
    // this stack. We will pop off the top one from the D frontier
    // set the implications.
    // On calling D again, few more gates will be added on the top of the D frontier.
    
    while ( !circuit.DFrontier.empty() )
    {
         
        ATPG_DFILE << "Current size of D = " << circuit.DFrontier.size() 
                   << " About to take " << checkIter->iwire->id << "  from D" << endl;
     
        // Take the first element off the D frontier
        DFront = (circuit.DFrontier).front();
        checkIter = &DFront;
        (circuit.DFrontier).pop_front();
        
        
        /*Selecting a gate from the DFrontier*/
        Gate* curGate=dynamic_cast<Gate*>(checkIter->iwire->input);
        Value cval = ControlValues[curGate->gtype];

        list<Wire*>::iterator inputIter = (curGate->inputs).begin();
        for (;inputIter != (curGate->inputs).end();inputIter++)
        {
            if ((*inputIter)->value == U)
            {

                Implication* newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                ATPG_DFILE << "Adding backward implication: " << (*inputIter)->id
                    <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
                newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                ATPG_DFILE << "Adding forward implication: " << (*inputIter)->id
                    <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
            }         
            else
              ATPG_DFILE << "Wire: " << (*inputIter)->id << "value: " <<  (*inputIter)->value <<endl; 


        }

      /* If the D-Drive succeeded through this gate, well and good, return true
       * Else we try the next gate in the D-frontier
       */
     if (D_Algo() == true) return true;
    }

// For J frontier, we need not pop off the element.
// When we set the implications, Imply_And_Check will remove the gate
// from the D frontier. This is the only difference between DFrontierWork and JFrontierWork
//
JFrontierWork:
    if (circuit.JFrontier.empty()) return true;
    checkIterator = (circuit.JFrontier).begin();

    PRINTJFRONTIER;
    
    //  Selecting a gate from the JFrontier
    //  A note about the J Frontier:
    //  We add a gate to the J Frontier if:
    //  1. Its output is c (control val) xor i
    //  2. more than input has value x
    //  3. none of the inputs which currently have
    //  known values have value c
    for (;checkIterator !=(circuit.JFrontier).end();checkIterator++)
    {
       Gate* curGate=dynamic_cast<Gate*>(checkIterator->iwire->input);
       Value cval = ControlValues[curGate->gtype];
        
      cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " ;
      cout<<__FILE__<<__LINE__ << "    " << "Selected gate: " << curGate->id 
           << "from the J Frontier" << endl;
      cout<<__FILE__<<__LINE__ << "    " << "Control value is: " << cval << endl;
      cout<<__FILE__<<__LINE__ << "    " << endl <<  endl;


      /*Select an input with Value U*/
      list<Wire*>::iterator inputIter = (curGate->inputs).begin();
      for (;inputIter != (curGate->inputs).end();inputIter++)
      {
          if (Compatible((*inputIter)->value,cval))
          {
              // All implications backward
              Implication* newImply= new Implication(*inputIter,(Value)(cval),false); /*bool false = 0 = backward*/
              cout<<__FILE__<<__LINE__ << "    " << "+++++++++++++++++++++ Implying CVal: " << cval <<   " on input: " <<  (*inputIter)->id << endl;
              (ImpliQueue).push(newImply);
              
              newImply= new Implication(*inputIter,(Value)(cval),true); /*bool false = 0 = backward*/
              (ImpliQueue).push(newImply);
              if (D_Algo() == true) return true;
              else 
                  // Implying c on this particular input didn't work
                  // so imply cbar 
              {
                  cout<<__FILE__<<__LINE__ << "    " << "+++++++++++++++++++++ Implying ~CVal: " <<  ((~cval)&0xf) <<   " on input: " <<  (*inputIter)->id << endl;
           
                  newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                  (ImpliQueue).push(newImply);

                  newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                  (ImpliQueue).push(newImply);
                  if (D_Algo() == true) return true;

              }
          }         
      }
      return false;
    }
    return false;
}



void ATPG::Update_PI_For_9V()
{
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        if ( ( (iter->second)->value == ZEROBYU )  ||  ( (iter->second)->value == UBYZERO ))
            (iter->second)->value  = ZERO;
        if (( (iter->second)->value == UBYONE )  ||  ( (iter->second)->value == ONEBYU ) )
            (iter->second)->value  = ONE;
    }
}






bool ATPG::Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value impliedValue)
{
   
 
    ATPG_DFILE << "Handling implication on wire " << curWire->id << endl;
    assert(curImplication->direction == true);

    // Check if the value of the wire is already set
    // And if it is the right value or not
    if (!Compatible(curWire->value,impliedValue))
    {
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }


    // If it is a PO, set the value to the wire 
    // and pop off the implication 
    if ((curWire->outputs).empty())
    {
        Change_Value_And_Update_Log(curImplication);
        ImpliQueue.pop();
        ATPG_DFILE<< "Success, po reached and po value set to:" << impliedValue << endl;
        return true;
    }


    Element *curEle = *((curWire->outputs).begin());

    // CASE 1: If there is a stemout, just
    // do the needful and return.

    if ( curEle->type ==  WIRE )
    {

        Implication* newImply; 
        ATPG_DFILE << "The forward implication is on a wire which is a stem" << endl;

        // First set the value of the current wire 
        // ie.the stem
        Change_Value_And_Update_Log(curImplication);



        list<Element*>::iterator iter=(curWire->outputs).begin();
        for (;iter != (curWire->outputs).end(); iter++)
        {
            Wire *owire = dynamic_cast<Wire*>(*iter);
            assert(owire != NULL);

            newImply = new Implication(owire, curImplication->value,true); 
            ATPG_DFILE << "Adding new implication on the branch" << owire->id  << " of stem " << curWire->id << endl;
            ImpliQueue.push(newImply); 
        }


        // pop off the current implication and return
        ATPG_DFILE << "Popping off the  implication on the stem " << endl;
        assert( ImpliQueue.front()->wire->id  == curWire->id);
        ImpliQueue.pop();
        return true;
    }

    // Case 2:  The wire has only one output
    // and it is a gate.

    Gate *curGate = dynamic_cast<Gate*>(curEle);
    assert(curGate);
    
    Value gateOldOutput = curGate->output->value;
    Value wireOldValue = curWire->value;

    // set the value for the wire, ie impliedValue to wire->value
    ATPG_DFILE << "Setting the value of the wire " << curImplication->wire->id  
     <<  " to the implied value " <<  curImplication->value << endl; 
    Change_Value_And_Update_Log(curImplication);
        

    // Now that the value of the wire implied on is set, evaluate the 
    // gate again.
    Value gateNewOutput = curGate->Evaluate();

    // if the old gate output is not unknown and 
    // the new value doesn't agree with old 
    // ( eg: 1 and u/0 don't agree, but D and u/0 or D and 1/u agree )
    //if ( (~(gateOldOutput & U)) && (gateNewOutput & geteOldOutput) )
    // if ( (gateOldOutput != U) && (gateNewOutput != U)  
    //       && ( ((gateNewOutput&gateOldOutput) != gateNewOutput) 
    //         ||   ((gateNewOutput|gateOldOutput) != gateOldOutput) ))
       
    if ((curGate->output != circuit.faultWire) && !Compatible(gateOldOutput,gateNewOutput))
    {
        // revert back !
        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
        cout<<__FILE__<<__LINE__ << "    " << "gateOldOutput: " << gateOldOutput << "gateNewOutput: " << gateNewOutput << endl;
        return false;
    }
    
    cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Debug: gateOldOutput = " << gateOldOutput << "new = " << gateNewOutput << endl;
    // Case - gate's old output is unknown or not completely known
    if ( isNotKnown(gateOldOutput) ) 
    {
        // Case I - gate old value == new value = unknown
        //if (gateNewOutput == U)
        if (isNotKnown(gateNewOutput))
        {
	    // add to the D frontier, only if there is error on the input line
	    if ( (impliedValue == D) || (impliedValue == DBAR))
	    {
		    // Add the gate to D frontier
		    cout<<__FILE__<<__LINE__ << "    " << "Adding gate to D frontier: " << curGate->id <<endl; 
		    cout<<__FILE__<<__LINE__ << "    " << "****************DEBUG***********"  << endl;
		    cout<<__FILE__<<__LINE__ << "    " << "Added gate to DFrontier with value: "  << gateNewOutput << endl;
		    Add_To_DFrontier(curGate->output, gateNewOutput);
	    }
            // Set the value of the value (useful in 9V)
            // curGate->output->value = gateNewOutput;
            // pop off the implication and proceed !
            ImpliQueue.pop();
            return true;
        }

        // Now just propage the value further
        // But before that check if the gate is in D frontier 
        // and remove the gate from it. Because it is resolved now

        if (Remove_From_D(curGate->output))
             cout<<__FILE__<<__LINE__ << "    " << "INFO: The gate is indeed in D and has been removed" << curGate->id << endl;
        else 
            cout<<__FILE__<<__LINE__ << "    " << "INFO: The gate is not there in D frontier. report from " << __LINE__ << "    " << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (ImpliQueue).pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << curGate->output->id << ". Line: " << __LINE__ << "    " << endl;
        (ImpliQueue).push(newImply); 

	return true;

    }

    // This is the case where the gate ouput is previously known.
    // Reason: Due to some implication, which finally added this 
    // gate to J frontier  
    else 
    {
        cout<<__FILE__<<__LINE__ << "    " << "Kashyap: Hey I am going to remove from J " << endl;
        if ( gateNewOutput == U )
        {
            // pop off the current implication
            (ImpliQueue).pop();
            return true;
        }

        if (Remove_From_J(curGate->output))
            cout<<__FILE__<<__LINE__ << "    " << "INFO: The gate is indeed in J and has been removed" << curGate->id << endl;
        else 
            cout<<__FILE__<<__LINE__ << "    " << "INFO: The gate is not there in J frontier. report from " << __LINE__ << "    " << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (ImpliQueue).pop();
//        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
//        cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << curGate->id << ". Line: " << __LINE__ << "    " << endl;
//        (ImpliQueue).push(newImply); 
        return true;
    }

    cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
    return false;

}






bool ATPG::Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value impliedValue)
{

    // First check if the curr value is compatable with current
    if ( (curWire != circuit.faultWire) && 
            (!Compatible(curWire->value, curImplication->value) 
            ) )
        return false;


    // Check if the wire is a PI.
    // If yes just set the value and
    // we are done with the implication 
    if ((curWire->input) == NULL)
    {
        cout<< "Reached PI." << endl;
        ImpliQueue.pop();
        if (curWire != circuit.faultWire) 
            Change_Value_And_Update_Log(curImplication);
        return true;
    }


    // Handle two cases. Fanout and no fanout.
    // CASE 1: If there is a stemout, just
    // do the needful and return.

    if ((curWire->input)->type == WIRE)
    {
        // first check if the curr value is compatable with current
        if ( (curWire != circuit.faultWire) && 
                (!Compatible(curWire->value, curImplication->value) 
                ) )
            return false;


        // First set the value of the current wire
        if (curWire != circuit.faultWire )
            Change_Value_And_Update_Log(curImplication);


        Wire *stemWire = dynamic_cast<Wire*> (curWire->input);


        // Add a new implication for the stem - backward
        Implication* newImply = new Implication(stemWire, curImplication->value,false); 
        (ImpliQueue).push(newImply); 

        // Add implications for the remaining branches of the input
        list<Element*>::iterator iter=(stemWire->outputs).begin();
        for (; iter != (stemWire->outputs).end(); iter++)
        {
            Wire *owire = dynamic_cast<Wire *> (*iter);

            // If this assert fails == resolve branches failure
            assert(owire != NULL);
            if ( owire != curWire )
            {
                // Add forward implications for the remaining branches, other than the current one
                newImply = new Implication(owire, curImplication->value,true);
                ImpliQueue.push(newImply); 
            }
        }

        // Pop off the current implication and return
        ImpliQueue.pop();
        return true;
    }


    // CASE 2:  Not fanout. So the wire is the output 
    // of a gate.

    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    Value xorValue =  Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]);

    assert(curGate);
    cout<< "Backward implication is on this gate's output: " << curGate->id << endl;

    if ( impliedValue == xorValue)
    {
        return Handle_Output_Coming_From_Control_Value(curImplication,curWire,impliedValue);
    }
    else if  ( impliedValue == Do_Xor((Value)((~ControlValues[curGate->gtype])&0xf),InversionValues[curGate->gtype]))

    {
        return Handle_Output_Coming_From_Noncontrol_Value(curImplication,curWire,impliedValue);
    }
    else 
    {

        // Note: The xor for the gates which have no
        // controlling value will always return U
        // Hence, this piece of code will be used
        // for BUF  NOT and XOR gates.

        // NOT & BUF GATE are almost identical
        if ((curGate->gtype == NOT) || (curGate->gtype == BUF))
        {

            // First take care of the output wire, on this
            // implication has been made.
            if (curWire != circuit.faultWire)
            {
                if ( !Compatible(curWire->value, curImplication->value) )
                    return false;
                Change_Value_And_Update_Log(curImplication);
            }


            // Then take care of the gate inputs.
            Wire *iwire = *((curGate->inputs).begin());
            Value negValue =  (Value)((~impliedValue)&0xf);

            cout << "Added new wire to Implication queue" << iwire->id << " Value: " << negValue << endl;
            Implication* newImply= new Implication(iwire,(curGate->gtype == NOT)?negValue:impliedValue,false);
            ImpliQueue.push(newImply);


            ImpliQueue.pop();
            return true;
        }
        else 

        { 
            // TODO: Handle XOR
            cout << "Gate type is :" << curGate->gtype << endl;
            cout<<__FILE__<<__LINE__ << "    " <<"Something wrong"<< endl;
            assert(false);
        }
    }
}



Implication* Find_In_Logs_List(Wire* wire)
{
    list<Implication*>:: iterator iter = Logs.begin();
    for (;iter!=Logs.end();iter++)
    {
         if (wire == (*iter)->wire) 
            return (*iter);
    }
    return NULL;
}

// TODO: Create a new class for log, which doesn't contain the bool direction
bool Change_Value_And_Update_Log (Implication *curImpli)
{
	// You have a intention, I make it true :) Enjoy !
	// But before I have to log it :P

	if (!curImpli->wire->modified)
	{

		// now it's modified
		curImpli->wire->modified = true;

		// Save the old value of the wire
		Value oldValue = (curImpli->wire)->value;

		// create a new implication
		Implication *newImpli = new Implication(curImpli->wire, oldValue, curImpli->direction);

		// push the new intention
		Logs.push_back(newImpli);

	}
	// change the curr value
	(curImpli->wire)->value = curImpli->value;


	return true;
}




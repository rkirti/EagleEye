#include "circuit.h"
#include <stdio.h>
#include "atpg.h"
#include "macros.h"
#include "dot.h"

ofstream ATPG_DFILE;
ofstream faultWriteFile;
ofstream randomVectorFile;

bool ATPG::Do_ATPG(Wire *faultwire, Value faultval)
{
    bool result;
    Implication* newImply;

    ATPG_DFILE << "ATPG called for faulty wire " <<  faultwire->id << endl;
    ATPG_DFILE << "Faulty value of the wire is " <<  faultval << endl;
   
    // set the fault wire
    (circuit.faultWire) = faultwire; 
    


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
    ATPG_DFILE << "Returning from D Algo for wire "  <<  faultwire->id << " with result " << result << endl;
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

    if ( (curWire != circuit.faultWire) && !Compatible(curWire->value,impliedValue))
    {
        ATPG_DFILE << "Implied value " << impliedValue << "  Current value  " << curWire->value << endl; 
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }


    // ACTION 2:
    // If the current value of the wire equals the 
    // implied value, the implication is popped here.
    // This means that either the gate is in Jfrontier already
    // or is already set by a forward impli
    if (curWire->value == impliedValue) 
    {
        ATPG_DFILE << "Implied value equals the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Popping off the implication" << endl;
        ATPG_DFILE << endl;
        ImpliQueue.pop();
        return true;
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

    if ( (curWire != circuit.faultWire) &&   !Compatible(curWire->value,impliedValue))
    {

        ATPG_DFILE << "Implied value " << impliedValue << "  Current value  " << curWire->value << endl; 
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }

    // ACTION 2:
    // If the current value of the wire equals the 
    // implied value, the implication is popped here.
    // This means that either the gate is in Jfrontier already
    // or is already set by a forward impli
    if (curWire->value == impliedValue) 
    {
        ATPG_DFILE << "Implied value equals the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Popping off the implication" << endl;
        ATPG_DFILE << endl;
        ImpliQueue.pop();
        return true;
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
        
        ATPG_DFILE << "Adding to J Frontier:  " << (*iter)->id << endl;
        Add_To_JFrontier(curWire,impliedValue);
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
    circuit.TempJFrontier.push_back(WireValuePair(wire,value));
    ATPG_DFILE  << "Added a gate with output wire " <<  wire->id << " to JFrontier." << endl;
    ATPG_DFILE <<  "JFrontier is now  " << endl; 
    PRINTJFRONTIER;
    ATPG_DFILE << endl << endl;
    return true;
}


bool ATPG::Add_To_DFrontier(Wire *wire,Value value)
{
    circuit.TempDFrontier.push_front(WireValuePair(wire,value));
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
            ATPG_DFILE  << "Removed a gate with output wire " <<  wire->id << " from DFrontier." << endl;
            ATPG_DFILE <<  "DFrontier is now  " << endl; 
            PRINTDFRONTIER;
            continue;   // we can return from function
        }
        iter ++;
    }
    if (!result) ATPG_DFILE << "Not found in DFrontier : gate with output wire " <<  wire->id << " from DFrontier." << endl;
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
    // Merge the temporary frontiers with the global ones.
    Merge_Frontiers();


    return true;

fail:
    Failure();
    return false;
}



void ATPG::Merge_Frontiers()
{
    list<WireValuePair>::iterator iter= circuit.TempJFrontier.begin();
    for (; iter != circuit.TempJFrontier.end();iter++)
        circuit.JFrontier.push_front(*iter);
    for (iter= circuit.TempDFrontier.begin(); iter != circuit.TempDFrontier.end();iter++)
        circuit.DFrontier.push_front(*iter);
    ATPG_DFILE << "Merged the temp.frontiers with global ones" << endl;

    circuit.TempJFrontier.clear();
    circuit.TempDFrontier.clear();

    ATPG_DFILE << "Cleared the temp.frontiers" << endl;

    return;
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
    // Don't remove the logs because they might be used by Jfrontier.
    // DfrontierWork clears the logs in the start.
//    Logs.clear();
    return;
}


void ATPG::Failure()
{
    ATPG_DFILE << "Imply_And_Check failed. NEED TO BACKTRACK" << endl << endl;
    ATPG_DFILE << "Using the logs to reverse all decisions/assignments" << endl;
    list<Implication*>::iterator iter= Logs.begin();
    for (; iter!=Logs.end();iter++)
    {
        ATPG_DFILE << "Reverting value of wire" << (*iter)->wire->id << " current val:" << (*iter)->wire->value << " reverting to " << (*iter)->value << endl; 
	    // Old value is on the RHS
	    (*iter)->wire->value = (*iter)->value;
	    // reset the modified flag anyways
	    (*iter)->wire->modified = false;
    }
    Logs.clear();
    
    // remove all the implications thought of
    while (!ImpliQueue.empty())
	    ImpliQueue.pop();
    circuit.TempJFrontier.clear();
    circuit.TempDFrontier.clear();

    ATPG_DFILE << "Cleared the temp.frontiers" << endl;
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
    // Error is not at PO. Algo should execute
    //
    // We don't need to save logs. Therefore let us clear the logs;
    Logs.clear();
    

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

        // Handle Xor here
        if (curGate->gtype == XOR)
        {
            ATPG_DFILE << "Handling Xor in D frontier" << endl;

            // Just lazy to handle mutiple input xor...For now, only 2 input xor
            if ((curGate->inputs).size() != 2)
                assert(false);

            if ((*inputIter)->value != U)
                inputIter++;

            if ((*inputIter)->value != U)
            {
                ATPG_DFILE << __LINE__ << "  Something wrong ! this gate should not be in D frontier" << endl;
                goto DoneXor;
            }

            // We got two bullets and if we can't shoot the balloon, we lose.
            
            // First attempt - Imply 0 on the unknown wire and observe
            // Second attempt - Imply 1 on the unknown wire and observe

            for (int i=0;i<2; i++)
            {
                // Pushing backward impli
                Implication* newImply= new Implication(*inputIter,(Value)(i*0xf),false); /*bool false = 0 = backward*/ // remember to push 0 or 15
                ATPG_DFILE << "Adding backward implication: " << (*inputIter)->id << " value "<< i*0xf << endl; 
                (ImpliQueue).push(newImply);

                // Pushing forward impli
                newImply= new Implication(*inputIter,(Value)(i*0xf),true); /*bool false = 0 = backward*/
                ATPG_DFILE << "Adding forward implication: " << (*inputIter)->id << " value "<< i*0xf << endl;
                (ImpliQueue).push(newImply);

                // If the D-Drive succeeded through this gate, well and good, return true
                // Else we have another trial - i e try ONE on the unknown wire 
                if (D_Algo() == true) return true;
            }

DoneXor:
            continue;
        }



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

    // We have tried all the gates, but couldn't propagate the error ( if we come here, it means that we couldn't propagate throught any gate in D frontier )
    // Hence we return false;
    return false;

// For J frontier, we need not pop off the element.
// When we set the implications, Imply_And_Check will remove the gate
// from the D frontier. This is the only difference between DFrontierWork and JFrontierWork
//
JFrontierWork:
    list<Implication *>CurLogs;
    CurLogs = Logs;
    Logs.clear();
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
       Gate* curGate=dynamic_cast<Gate*>(checkIterator->iwire->input);
       Value cval = ControlValues[curGate->gtype];
        
      ATPG_DFILE <<__FILE__<<__LINE__ << "    " << __LINE__ << "    " ;
      ATPG_DFILE <<__FILE__<<__LINE__ << "    " << "Selected gate: " << curGate->id 
        << "from the J Frontier" << endl;
      ATPG_DFILE <<__FILE__<<__LINE__ << "    " << "Control value is: " << cval << endl;
      ATPG_DFILE <<__FILE__<<__LINE__ << "    " << endl <<  endl;


      /*Select an input with Value U*/
      list<Wire*>::iterator inputIter = (curGate->inputs).begin();

      // Handle Xor here
      if (curGate->gtype == XOR)
      {
          ATPG_DFILE << "Handling Xor in J frontier" << endl;

          // Just lazy to handle mutiple input xor...For now, only 2 input xor
          if ((curGate->inputs).size() != 2)
              assert(false);

          // iterator to the second input wire
          list<Wire*>::iterator inputIter2 = ++((curGate->inputs).begin());

          ATPG_DFILE << "in1 = " << ((*inputIter)->value) << " in2 = " << ((*inputIter2)->value) << " want to justify value=" << checkIterator->value << endl;

          // Case I - both the inputs are unknown
          if  ( ((*inputIter)->value == U) && ((*inputIter2)->value == U) )
          {
              // Here we try the two solutions to satisfy the
              // output value

              Value in1,in2;

              // I hope that the implied value is either 0 or 1 and
              // not other values.
              if (checkIterator->value == ONE)
              {
                  in1 = ONE; in2 = ZERO;
              }
              else 
                  in1 = in2 = ZERO;

              // trying the two ways
              for (int i=0;i<2; i++)
              {
                  // Pushing backward impli on in1
                  Implication* newImply= new Implication(*inputIter,in1,false); /*bool false = 0 = backward*/
                  ATPG_DFILE << "Adding backward implication: " << (*inputIter)->id << " value "<< i << endl; 
                  (ImpliQueue).push(newImply);

                  // Pushing forward impli on in1
                  newImply= new Implication(*inputIter,in1,true); /*bool false = 0 = backward*/
                  ATPG_DFILE << "Adding forward implication: " << (*inputIter)->id << " value "<< i << endl;
                  (ImpliQueue).push(newImply);

                  // Pushing backward impli on in2
                  newImply= new Implication(*inputIter2,in2,false); /*bool false = 0 = backward*/
                  ATPG_DFILE << "Adding backward implication: " << (*inputIter2)->id << " value "<< i << endl; 
                  (ImpliQueue).push(newImply);

                  // Pushing forward impli on in2
                  newImply= new Implication(*inputIter2,in2,true); /*bool false = 0 = backward*/
                  ATPG_DFILE << "Adding forward implication: " << (*inputIter2)->id << " value "<< i << endl;
                  (ImpliQueue).push(newImply);

                  // If the D-Drive succeeded through this gate, well and good, return true
                  // Else we have another trial - i e try ONE on the unknown wire 
                  if (D_Algo() == true) return true;

                  // Compliment the original values
                  in1 =(Value) ((~(int)in1)&0xf);
                  in2 =(Value) ((~(int)in2)&0xf);
              }
          }
          // Case II - Only one wire is unknown
          else 
          {
              // Here inputIter should point to the first wire and inputIter2 to second
              // Make sure iter2 points to the wire with known value and iter to unknown
              if ((*inputIter2)->value == U)
              {
                  inputIter2--;
                  inputIter++;
              }
              
              // TODO:put some error checks

              // Error check 1: if we have d or dbar on any inputs, we cannot justify a full value(0 or 1) on the output of an xor
#define       ISDORDBAR(X)  ((X == D) || (X == DBAR))
              if (ISDORDBAR((*inputIter)->value) || ISDORDBAR((*inputIter2)->value))
                  return false;

#define       V_XOR(x,y)     (Value)((((int)(x) & (~(int)(y))) | ( (~(int)(x)) & (int)(y)))&0xf)

              Value val = V_XOR( checkIterator->value, (*inputIter2)->value);

              // Pushing backward impli
              Implication* newImply= new Implication(*inputIter,val,false); /*bool false = 0 = backward*/
              ATPG_DFILE << "Adding backward implication: " << (*inputIter)->id << " value "<< val << endl; 
              (ImpliQueue).push(newImply);

              // Pushing forward impli
              newImply= new Implication(*inputIter,val,true); /*bool false = 0 = backward*/
              ATPG_DFILE << "Adding forward implication: " << (*inputIter)->id << " value "<< val << endl;
              (ImpliQueue).push(newImply);

              // If the D-Drive succeeded through this gate, well and good, return true
              // Else fail 
              if (D_Algo() == true) return true;
          }

          goto JFrontierFail;
      }

      for (;inputIter != (curGate->inputs).end();inputIter++)
      {
          if (Compatible((*inputIter)->value,cval))
          {
              // All implications backward
              Implication* newImply= new Implication(*inputIter,(Value)(cval),false); /*bool false = 0 = backward*/
              ATPG_DFILE <<__FILE__<<__LINE__ << "    " << "+++++++++++++++++++++ Implying CVal: " << cval <<   " on input: " <<  (*inputIter)->id << endl;
              (ImpliQueue).push(newImply);
              
              newImply= new Implication(*inputIter,(Value)(cval),true); /*bool false = 0 = backward*/
              (ImpliQueue).push(newImply);
              if (D_Algo() == true) return true;
              else 
                  // Implying c on this particular input didn't work
                  // so imply cbar 
              {
                  ATPG_DFILE <<__FILE__<<__LINE__ << "    " << "+++++++++++++++++++++ Implying ~CVal: " <<  ((~cval)&0xf) <<   " on input: " <<  (*inputIter)->id << endl;
           
                  newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                  (ImpliQueue).push(newImply);

                  newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                  (ImpliQueue).push(newImply);
                  //if (D_Algo() == true) return true;

              }
          }         
      }
JFrontierFail:
      // before we fail, we should undo the values we set
      Logs = CurLogs;
      Failure();
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
   
 
    ATPG_DFILE << "Handling forward implication on wire " << curWire->id << endl;
    assert(curImplication->direction == true);

    // Check if the value of the wire is already set
    // And if it is the right value or not
    if ( (curWire != circuit.faultWire) && !Compatible(curWire->value,impliedValue))
    {

        ATPG_DFILE << "Implied value " << impliedValue << "  Current value  " << curWire->value << endl; 
        ATPG_DFILE << "Implied value contradicts the current value of the wire." << endl;
        ATPG_DFILE << "Wire in question is :" << curWire->id << endl;
        ATPG_DFILE << "Returning false , need to backtrack" << endl;
        ATPG_DFILE << endl;
        return false;
    }

    // CASE 0:
    // If it is a PO, set the value to the wire 
    // and pop off the implication 
    if ((curWire->outputs).empty())
    {
        Change_Value_And_Update_Log(curImplication);
        ImpliQueue.pop();
        ATPG_DFILE << "Success, po reached and po value set to:" << impliedValue << endl;
        return true;
    }


    Element *curEle = *((curWire->outputs).begin());

    // CASE 1: If there is a stemout, just
    // do the needful and return.
    
    if ( curEle->type ==  WIRE )
        return Handle_Forward_Implication_On_Stem(curImplication,curWire,impliedValue);

    // Case 2:  The wire has only one output
    // and it is a gate.

    Gate *curGate = dynamic_cast<Gate*>(curEle);
    assert(curGate);
    
    Value gateOldOutput = curGate->output->value;
    Value wireOldValue = curWire->value;

    // set the value for the wire, ie impliedValue to wire->value
    ATPG_DFILE << "Setting the value of the wire " << curImplication->wire->id  
               << " to the implied value " <<  curImplication->value << endl; 
    Change_Value_And_Update_Log(curImplication);
        

    // Now that the value of the wire implied on is set, evaluate the 
    // gate again.
    Value gateNewOutput = curGate->Evaluate();



    // This is *some* logic. (The use of the || instead of just checking
    // for the first condition)
    // TODO: @Kashyap: Please put explanatory comments :-P


    if ((curGate->output != circuit.faultWire) && !(Compatible(gateOldOutput,gateNewOutput)|| Compatible(gateNewOutput,gateOldOutput)))
    {
        // revert back !
        ATPG_DFILE << "Gate's old output, without setting the value of the wire was" << gateOldOutput << endl;
        ATPG_DFILE << "Gate's new output, after setting the value of the wire is" << gateNewOutput << endl;
        ATPG_DFILE << "Not compatible,Returning false" << endl;
        return false;
    }
    
    ATPG_DFILE << "GateOldOutput = " << gateOldOutput << " NewOutput = " << gateNewOutput << endl;
    
    
    
    // Case - gate's old output is unknown or not completely known
    if ( isNotKnown(gateOldOutput) ) 
    {
        // Case I - gate old value == new value = U / partially known
        if (isNotKnown(gateNewOutput))
        {
            // Add to the D frontier, only if there is error on the input line
            if ( (impliedValue == D) || (impliedValue == DBAR))
            {
                // Add the gate to D frontier
                 ATPG_DFILE << "Adding gate to D frontier: " << curGate->id  
                            << " with value: "  << gateNewOutput << endl;
                 //bug: Add_To_DFrontier(curGate->output, gateNewOutput);
                 Add_To_DFrontier(curGate->output, U);
            }

            // Set the value of the value (useful in 9V)
            // curGate->output->value = gateNewOutput;
            // pop off the implication and proceed !
            ImpliQueue.pop();
            return true;
        }

        // Now just propagate the value further
        // But before that check if the gate is in D frontier 
        // and remove the gate from it. Because it is resolved now

        if (Remove_From_D(curGate->output))
             ATPG_DFILE << "The gate is indeed in D and has been removed" << curGate->id << endl;
        else 
            ATPG_DFILE << "The gate is not there in D frontier. report from " << __LINE__ << endl;
                

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        ImpliQueue.pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        ATPG_DFILE << "Adding implication on wire " << curGate->output->id << "  Line: " << __LINE__ << "    " << endl;
        ImpliQueue.push(newImply); 

	return true;

    }

    // This is the case where the gate ouput is previously known.
    // Reason: Due to some implication, which finally added this 
    // gate to J frontier  
    else 
    {
       ATPG_DFILE << "Removing from J frontier" << endl;
        if ( gateNewOutput == U )
        {
            // pop off the current implication
            (ImpliQueue).pop();
            return true;
        }

        if (Remove_From_J(curGate->output))
            ATPG_DFILE << "The gate is indeed in J and has been removed  " << curGate->id << endl;
        else 
            ATPG_DFILE << "The gate is not there in J frontier. report from " << __LINE__  << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        ImpliQueue.pop();
//        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
//        cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << curGate->id << ". Line: " << __LINE__ << "    " << endl;
//        (ImpliQueue).push(newImply); 
        return true;
    }

    ATPG_DFILE<< "Returning false from end of function Resolve_Forward_Implication" << endl;
    return false;

}


bool  ATPG::Handle_Forward_Implication_On_Stem(Implication* curImplication, Wire* curWire, Value impliedValue)
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


    // Pop off the current implication and return
    ATPG_DFILE << "Popping off the implication on the stem " << endl;
    assert(ImpliQueue.front()->wire->id  == curWire->id);
    ImpliQueue.pop();
    return true;
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
    ATPG_DFILE << "Backward implication is on this gate's output: " << curGate->id << endl;

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
	else if (curGate->gtype == XOR)
        {
            ATPG_DFILE << __FILE__ << __LINE__ << "Handling xor :)" << endl;

            // Just lazy to handle mutiple input xor...For now, only 2 input xor
            if ((curGate->inputs).size() != 2)
                assert(false);

            // iterators to the two wires
            list<Wire*>::iterator inputIter, inputIter2 = (curGate->inputs).begin();
            inputIter = inputIter2++;

            // first reflect the impli on the wire
            if (curWire != circuit.faultWire)
            {
                if ( !Compatible(curWire->value, curImplication->value) )
                    return false;
                Change_Value_And_Update_Log(curImplication);
            }
            
            // Case I - both the inputs are unknown. Then add the gate to J frontier
            if  ( ((*inputIter)->value == U) && ((*inputIter2)->value == U) )
            {
                Add_To_JFrontier(curWire, curImplication->value);
                ImpliQueue.pop();
                return true;
            }
            // Case II - when both the values are known, then check for consistency
            // and return t/f accordingly
            else if  ( ((*inputIter)->value != U) && ((*inputIter2)->value != U) )
            {
                // if the gate evaluates to different value than the implied one,
                // report error
                if ( !Compatible(curGate->Evaluate(), curImplication->value) )
                    return false;

                // else pop the impli and return true
                ImpliQueue.pop();
                return true;
            }
            // Case III - when one of the values is not known
            // Then add new implications to propagate backward
            else 
            {
                // Here inputIter should point to the first wire and inputIter2 to second
                // Make sure iter2 points to the wire with known value and iter to unknown
                if ((*inputIter2)->value == U)
                {
                    inputIter2--;
                    inputIter++;
                }

                // if the known input is not 0 or 1 (i.e D or Dbar), then the
                // backward implication can never be satisfifed.
                if (((*inputIter2)->value != ONE) && ((*inputIter2)->value != ZERO))
                {
                    ImpliQueue.pop();    // not needed
                    return false;
                }

#define         V_XOR(x,y)     (Value)((((int)(x) & (~(int)(y))) | ( (~(int)(x)) & (int)(y)))&0xf)

                Value val = V_XOR( curImplication->value, (*inputIter2)->value);

                // pop off the current impli and push new ones
                (ImpliQueue).pop();

                // Pushing backward impli
                Implication* newImply= new Implication(*inputIter,val,false); /*bool false = 0 = backward*/
                ATPG_DFILE << "Adding backward implication: " << (*inputIter)->id << " value "<< val << endl; 
                (ImpliQueue).push(newImply);

                // Pushing forward impli
                newImply= new Implication(*inputIter,val,true); /*bool false = 0 = backward*/
                ATPG_DFILE << "Adding forward implication: " << (*inputIter)->id << " value "<< val << endl;
                (ImpliQueue).push(newImply);
                return true;
            }

        }
        else 
        { 
            cout << "Gate type is :" << curGate->gtype << endl;
            cout << "Value implied is " << impliedValue << endl;
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

RandomVectorTest::RandomVectorTest()
{
    // open the rand vectors file for Generate and set random vectors functions to use
    randomVectorFile.open("tests/randvectors.txt",ios::out);
    if (!randomVectorFile.good())
    {
        cout << "Couldn't open the file tests/randvectors.txt" << endl;
        exit(-1);
    }

    // set the random seed for random vector generation
    srand(time(NULL));
}

RandomVectorTest::~RandomVectorTest()
{
    randomVectorFile.close();
}

void Generate_Full_FaultSet()
{
    faultWriteFile.open("tests/faults.txt",ios::out);
    if (!faultWriteFile.good())
    {
        cout << "Couldn't open the file tests/faults.txt" << endl;
        exit(-1);
    }

    map<string,Wire *>::iterator iter= circuit.Netlist.begin();
    string DFault =  " 0";
    string DBarFault  = " 1";

    for (; iter != circuit.Netlist.end(); iter++)
    {
        faultWriteFile << iter->second->id << DFault << endl; 
        faultWriteFile << iter->second->id << DBarFault << endl; 
    }
    faultWriteFile.close();
    return;
}


void RandomVectorTest::GenerateAndSetRVector()
{
    map<string,Wire *>::iterator iter;
    int randLen = sizeof(int)*8;
    int randNo = rand();
    int curPos = 0;
    for (iter = circuit.PriInputs.begin(); iter != circuit.PriInputs.end(); iter++)
    {
        bool bit = ( (randNo & (1 << curPos)) >> curPos);
        randomVectorFile << bit;
        (iter->second)->value = (Value)(bit*0xf);
        curPos++;
        if (curPos == randLen)
            curPos = 0;
    }
    randomVectorFile << endl;

    return;
}

void RandomVectorTest::PerformTest()
{
    int no_times = 1;
    
    cout << "The size of the fault set is " << (circuit.FaultSet).size() << endl;
    while (no_times--)  // need to change this to x% coverage && time limit
    {
        GenerateAndSetRVector();
        Value FaultWireOrigVal = U;

        vector<Value> faultFreeOutput,faultyOutput;

        // Evaluate the circuit without faults and store the bitmap;
        circuit.Evaluate();
        // Capture the fault free output
        faultFreeOutput = circuit.CaptureOutput();

        // Now insert each fault and test if it is detected or not
        list<Fault>::iterator iter = (circuit.FaultSet).begin();
        while (iter != (circuit.FaultSet).end())
        {
            circuit.Clear_Internal_Wire_Values();
            FaultWireOrigVal = iter->FaultSite->value;
            iter->FaultSite->value = (iter->faultType == 0)?ZERO:ONE;
            circuit.Evaluate();
            faultyOutput.clear();
            faultyOutput = circuit.CaptureOutput();
            iter->FaultSite->value = FaultWireOrigVal;

            if (faultyOutput != faultFreeOutput)
            {
                iter = (circuit.FaultSet).erase(iter);
                continue;
            }
            iter++;
        }
    }

    cout << "The size of the fault set is " << (circuit.FaultSet).size() << endl;

}


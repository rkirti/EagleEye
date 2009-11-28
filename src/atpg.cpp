#include "circuit.h"
#include "atpg.h"


static list<Implication*> Intentions;
static queue<Implication*> ImpliQueue;

bool ATPG::Do_ATPG()
{
   Value newVal = D;
   bool result;
   (circuit.faultWire) = ((circuit.Netlist).find("N10")->second);

    // n10 - backward ONE 
   Implication* newImply= new Implication(circuit.faultWire,ONE,false); 
   (ImpliQueue).push(newImply); 

    // n10 - forward D 
   newImply= new Implication(circuit.faultWire,D,true); 
   (ImpliQueue).push(newImply); 

//    // n0 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N0"))->second,ZERO,false); 
//    (ImpliQueue).push(newImply); 
//    // n1 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N1"))->second,ZERO,false); 
//    (ImpliQueue).push(newImply); 
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


// Total mess !! plz decorate it :)
bool ATPG::Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value curValue)
{
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert(Translate_Value_To_Int
            (curValue) == (Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype])));

    list<Wire*>::iterator iter = (curGate->inputs).begin();
    int inputUCount = 0;

    for (;iter != (curGate->inputs).end();iter++)
    {
        if  (Check_Wire_Value((*iter),U)) inputUCount++;
    }


    // TODO: The we need to propagate further
    if (Check_Wire_Value(curWire,curValue)) 
    {
        cout << "Curvalue is what we want. Implication resolved by itself" << endl;
        (ImpliQueue).pop();
        return true;
    } 

    if (Check_Wire_Value(curWire,U)) 
    {
        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }

    /*Check if all inputs of the gate are unknown or cbar*/
    /*TODO:You might want to iterate through the input list
     * instead. */
    if ( curGate->Evaluate() == curValue)
    {
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
	    Intentions.push_back(curImplication);
        (ImpliQueue).pop();
        return true;
    } 

    /*Some input has value c */
    if (curGate->Evaluate() != U) 
    {

        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }


    if (inputUCount>1) 
    {
        //(*iter)->value = curValue; <-- DOUBT: ?
        // if it is the faulty wire, don't set the value
        if ( curWire != (circuit.faultWire) )
            Intentions.push_back(curImplication);
        cout << "Adding to J Frontier:  " << (*iter)->id << endl;
        Add_To_JFrontier((*iter),curValue);
        (ImpliQueue).pop();
        return true;
    }

    if (inputUCount == 1)
    {
        for (iter = (curGate->inputs).begin();iter != (curGate->inputs).end();iter++)
        {
            if (Check_Wire_Value((*iter),U))
            {

                Value newVal = (ControlValues[curGate->gtype]);
                Implication* newImply= new Implication(*iter,newVal,false); /*bool false = 0 = backward*/
                cout << "Adding implication :  " << (*iter)->id 
                    << "value:  " <<  newVal << endl;
                (ImpliQueue).push(newImply); 

            }
        }

        (ImpliQueue).pop();
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
        Intentions.push_back(curImplication);
	return true;

    }

}


bool  ATPG::Check_Wire_Value(Wire* wire,Value val)
{
    bool flag=false;
    list<Implication*>::iterator iter= Intentions.begin();
    for (; iter!=Intentions.end();iter++)
    {
        if ((*iter)->wire->id == wire->id)
        {
            if (Compatible((*iter)->wire->value,val))
            {
                (*iter)->value = val; 
                return true;         
            }
            else 
            {
                cout <<"Contradiction:  prev." <<  (*iter)->wire->value << " new: " << val << endl;
                return false;
            }
         } 

    }

    if (Compatible(wire->value,val))
        return true;
   return false;
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









bool ATPG::Handle_Output_Coming_From_Noncontrol_Value(Implication* curImplication, Wire* curWire,Value curValue)
{
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert( Translate_Value_To_Int(curValue) == Do_Xor(!ControlValues[curGate->gtype],InversionValues[curGate->gtype]));
    assert(curGate);


    /* Check if the gate output value is either unknown
     * or what we want i.e. cbar xor i
     */

    /* This implication resolved if wire is already what we want */
    if (curWire->value == curValue) 
    {
        cout << "Curvalue is what we want. Implication resolved by itself"
            << endl;
        (ImpliQueue).pop();
        // At this point, justifying the inputs may be pending
        // hence we dont return here
    } 

    if (curWire->value != U) 
    {
        cout << "curWire->value is : " << curWire->value << endl;
        cout << "Returning false" << endl;
        return false;
    }


    /*Check if all inputs of the gate are unknown or cbar*/
    /*TODO:You might want to iterate through the input list
     * instead. 
     */


    if (curGate->Evaluate() == curValue)
    {
        cout << "Gate evals to what we want. Implication to be resolved "
            << endl;
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
	     curWire->value = curValue;
        (ImpliQueue).pop();
        return true;
    } 



    
    
    switch (curGate->Evaluate())
    {   
             case U:
            {
                list<Wire*>::iterator iter = (curGate->inputs).begin();
                for (;iter != (curGate->inputs).end();iter++)
                {
                    // All inputs should be cbar
                    Value newVal = (Value)(!(ControlValues[curGate->gtype]));
                    Implication* newImply= new Implication(*iter,newVal,false); /*bool false = 0 = backward*/
                    cout << "Adding implication :  " << (*iter)->id 
                        << "value:  " <<  newVal << endl;
                    (ImpliQueue).push(newImply); 
                }
		// if it is the faulty wire, don't set the value
		if ( curWire != (circuit.faultWire) )
		    curWire->value = curValue;
		(ImpliQueue).pop();
                return true;
            }


        default:
            /*Some input has value c */
            {

                cout << "Evaluate of " << curGate->id << " gives:" << curGate->Evaluate() << endl;
                cout << __LINE__ << endl ;
                cout << "Returning false" << endl;
                return false;
            }
    }

}




bool ATPG::Add_To_JFrontier(Wire *wire,Value value)
{
    circuit.JFrontier.push_back(WireValuePair(wire,value));
    return true;
    
}
bool ATPG::Add_To_DFrontier(Wire *wire,Value value)
{
    circuit.DFrontier.push_back(WireValuePair(wire,value));
    return true;
    
}



bool ATPG::RemoveFromD(Wire *wire)
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

    return result;
}



bool ATPG::RemoveFromJ(Wire *wire)
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

    return result;
}



bool ATPG::Imply_And_Check()
{
    cout << "Imply_And_Check called" << endl;
    
    assert(Intentions.empty());



    while (!ImpliQueue.empty())
    {

        //  Step 1: Get Details of current implication 
        Implication*  curImplication = (ImpliQueue).front();
        Wire* curWire = curImplication->wire;
        Value curValue = curImplication->value;
        cout << __LINE__ << ": ";
        cout << "New implication from the queue:  " << (curWire->id) <<  ":   "<< (curValue) ;
        if (curImplication->direction == 0) cout <<   "  backwards" << endl;
        else cout << "  forward" << endl;
            
        //  Resolving backward implication
        if (curImplication->direction == false) 
        {
            if (Resolve_Backward_Implication(curImplication,curWire,curValue) == false)
            {

                cout << __LINE__ << ": Returning false" << endl;
                return false;
            }
                else continue;

        }
        //  Resolving forward implication
        else
        {
            if (Resolve_Forward_Implication(curImplication,curWire,curValue) == false)
         {

                cout << __LINE__ << ": Returning false" << endl;
                return false;
         
         }
                else continue;

        }

    }

    Make_Assignments();
    return true;
}


void ATPG::Make_Assignments()
{
   list<Implication*>::iterator iter= Intentions.begin();
   for (; iter!=Intentions.end();iter++)
    {
        // Intended value is on the RHS
        (*iter)->wire->value = (*iter)->value;
    }
}



void ATPG::Failure()
{
    Intentions.clear();
    while (!ImpliQueue.empty())
        ImpliQueue.pop();
}



bool ATPG::D_Algo()
{ 
     //Ensure all implications are made and they 
     // are consistent
    if (Imply_And_Check() == false) 
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
                
                Implication* newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                cout << __LINE__  << ":  " ;
                cout << "Adding backward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
                newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                cout << __LINE__  << ":  " ;
                cout << "Adding forward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
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
              
                (ImpliQueue).push(newImply);
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


bool ATPG::Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
{
    assert(curImplication->direction ==true);

    // if the value of the wire is already set
    // And if it is the right value or not
    if ( (curWire->value != U) && (curWire->value != curValue) )
    {
        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }

    // if it is a PO, set the value to the wire 
    // and pop off the implication 
    if ((curWire->outputs).empty())
    {
        // set the value of the current wire
        curWire->value = curValue;
        (ImpliQueue).pop();
        cout << __LINE__ << ": Success, po reached and po value set to:" << curValue << endl;
        return true;
    }



    // Handle the fanout case
    /*
       if ( curEle->type ==  WIRE )
       {
       Wire* iwire = dynamic_cast<Wire*> curEle;
       (ImpliQueue).push(newImply); 
    // do something for the wire 
    //
    }*/

    /*Temporarily avoid fanout*/
    
    Element *curEle = *((curWire->outputs).begin());
    Gate *curGate = dynamic_cast<Gate*>(curEle);
    
    Value gateOldOutput = curGate->output->value;
    Value wireOldValue = curWire->value;

    // set the value for the wire, ie curValue to wire->value
    curWire->value = curValue;
    Value gateNewOutput = curGate->Evaluate();

    // if the old gate output is not unknown and 
    // the new value doesn't agree with old 
    // ( eg: 1 and u/0 don't agree, but D and u/0 or D and 1/u agree )
    //if ( (~(gateOldOutput & U)) && (gateNewOutput & geteOldOutput) )
    if ( (gateOldOutput != U) && (gateNewOutput != U)  
            && ( ((gateNewOutput&gateOldOutput) != gateNewOutput) 
                ||   ((gateNewOutput|gateOldOutput) != gateOldOutput) ))
    {
        // revert back !
        curWire->value = wireOldValue;
        cout << __LINE__ << ": Returning false" << endl;
        return false;
    }
    
    cout << __LINE__ << ": Debug: gateOldOutput = " << gateOldOutput << "new = " << gateNewOutput << endl;
    // Case - gate's old output is unknown or not completely known
    if ( isNotKnown(gateOldOutput) ) 
    {
        // Case I - gate old value == new value = unknown
        //if (gateNewOutput == U)
        if (isNotKnown(gateNewOutput))
        {
            // Add the gate to D frontier
            cout << "Adding gate to D frontier: " << curGate->id <<endl; 
            Add_To_DFrontier(curGate->output, gateNewOutput);
            // Set the value of the value (useful in 9V)
            // curGate->output->value = gateNewOutput;
            // pop off the implication and proceed !
            ImpliQueue.pop();
            return true;
        }

        // Now just propage the value further
        // But before that check if the gate is in D frontier 
        // and remove the gate from it. Because it is resolved now

        if (RemoveFromD(curGate->output))
           ;
            // cout << "INFO: The gate is indeed in D and has been removed" << curGate->id << endl;
        else 
            ;
            //cout << "INFO: The gate is not there in D frontier. report from " << __LINE__ << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (ImpliQueue).pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        cout << "Adding implication :  " << curGate->output->id << ". Line: " << __LINE__ << endl;
        (ImpliQueue).push(newImply); 

	return true;

    }

    // This is the case where the gate ouput is previously known.
    // Reason: Due to some implication, which finally added this 
    // gate to J frontier  
    else 
    {
        if ( gateNewOutput == U )
        {
            // pop off the current implication
            (ImpliQueue).pop();
            return true;
        }

        if (RemoveFromJ(curGate->output))
            cout << "INFO: The gate is indeed in J and has been removed" << curGate->id << endl;
        else 
            cout << "INFO: The gate is not there in J frontier. report from " << __LINE__ << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (ImpliQueue).pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        cout << "Adding implication :  " << curGate->id << ". Line: " << __LINE__ << endl;
        (ImpliQueue).push(newImply); 
        return true;
    }

    cout << __LINE__ << ": Returning false" << endl;
    return false;

}






bool ATPG::Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
{

    // Check if the wire is a PI.
    // If yes just set the value and
    // we are done with the implication 
    if ((curWire->input) == NULL)
    {
        cout << "Reached PI." << endl;
        curWire->value = curValue;
        (ImpliQueue).pop();
        return true;
    }

    // Handle two cases. Fanout and no fanout. Right now,
    // we evade the fanout case. Hence the assert.
    // Each wire has only a gate for an input.
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert(curGate);
    
    cout << "Backward implication is on this gate's output: " << curGate->id << endl;
    cout << "c xor i is  "<<  Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]) <<  endl;
    cout << "Value to be implied is : " <<  curValue << endl;


    if ( Translate_Value_To_Int(curValue) == Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]))
    {
        return Handle_Output_Coming_From_Control_Value(curImplication,curWire,curValue);
    }
    else 
    {
       return Handle_Output_Coming_From_Noncontrol_Value(curImplication,curWire,curValue);
    }
}




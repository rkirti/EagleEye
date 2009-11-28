#include "circuit.h"
#include <stdio.h>
#include "atpg.h"

static int atpgdebug=1;
extern FILE* ATPG_DFILE;

#define ATPGPRINT(...) { if (atpgdebug) fprintf(__VA_ARGS__); fprintf(ATPG_DFILE,"\n"); }
#define PRINTDFRONTIER   {  list<WireValuePair>::iterator iter= circuit.DFrontier.begin();   \
                            fprintf(ATPG_DFILE,"Printing DFrontier\n");      \
                            for (;iter!= circuit.DFrontier.end();iter++)  \
                            {      fprintf(ATPG_DFILE,"Wire: %s   Value: %d\n", (((*iter).iwire)->id).c_str(), (*iter).value);    \
                                   fprintf(ATPG_DFILE,"DFrontier printed\n");      \
                         } }

#define PRINTJFRONTIER   { list<WireValuePair>::iterator iter= circuit.JFrontier.begin();   \
                            fprintf(ATPG_DFILE,"Printing JFrontier\n");      \
                            for (;iter!= circuit.JFrontier.end();iter++)  \
                            {      fprintf(ATPG_DFILE,"Wire: %s   Value: %d\n", (((*iter).iwire)->id).c_str(), (*iter).value);    \
                                   fprintf(ATPG_DFILE,"JFrontier printed\n");      \
                         } }







static list<Implication*> Intentions;
static queue<Implication*> ImpliQueue;





bool ATPG::Do_ATPG()
{
   Value newVal = D;
   bool result;
   (circuit.faultWire) = ((circuit.Netlist).find("c_NAND3_1")->second); 	
 //  (circuit.faultWire) = ((circuit.Netlist).find("N3")->second); 	// Yay ! works now..... used to cup previously (for c17)

    // n10 - backward ZERO
   Implication* newImply= new Implication(circuit.faultWire,ZERO,false); 
   (ImpliQueue).push(newImply); 

    // n10 - forward DBAR 
   newImply= new Implication(circuit.faultWire,DBAR,true); 
   (ImpliQueue).push(newImply); 

//    // n0 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N0"))->second,ZERO,false); 
//    (ImpliQueue).push(newImply); 
//    // n1 - backward 0
//   newImply= new Implication(((circuit.Netlist).find("N1"))->second,ZERO,false); 
//    (ImpliQueue).push(newImply); 
////bool false = 0 = backward


   cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << "N10" 
    << "value:  " <<  newVal;
   result = D_Algo();
   
   
   
   Update_PI_For_9V();
   
   cout<<__FILE__<<__LINE__ << "    " << "Returning " << result << endl;
   map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    cout<<__FILE__<<__LINE__ << "    " << "Test vectors are : " << endl;
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        cout<<__FILE__<<__LINE__ << "    " << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

   iter =  (circuit.PriOutputs).begin();
    cout<<__FILE__<<__LINE__ << "    " << "Output vectors are : " << endl;
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        cout<<__FILE__<<__LINE__ << "    " << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

   iter =  (circuit.Netlist).begin();
    cout<<__FILE__<<__LINE__ << "    " << "Netlist : " << endl;
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        cout<<__FILE__<<__LINE__ << "    " << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }
}


// Total mess !! plz decorate it :)
bool ATPG::Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value curValue)
{
    cout<<__FILE__<<__LINE__ << "    " << "$$$$$$$$$$$$$$$$$$$Came here$$$$$$$$$$$$$$$$$$$$$" << endl;
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert(curValue == Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]));

    list<Wire*>::iterator iter = (curGate->inputs).begin();
    int inputUCount = 0;

    for (;iter != (curGate->inputs).end();iter++)
    {
        if  ( (*iter)->value  == U) inputUCount++;
    }


    // TODO: The we need to propagate further
    if (curWire->value == curValue) 
    {
        cout<<__FILE__<<__LINE__ << "    " << "Curvalue is what we want. Implication resolved by itself" << endl;
        (ImpliQueue).pop();
        return true;
    } 

    if (!Check_Wire_Value_For_Assignment(curWire,curValue)) 
    {
        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
        return false;
    }

    /*Check if all inputs of the gate are unknown or cbar*/
    /*TODO:You might want to iterate through the input list
     * instead. */
    // better use compatable function , instead of == 
    if (curGate->Evaluate() == curValue)
    {
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
    {
        cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
        Intentions.push_back(curImplication);
    }   
        (ImpliQueue).pop();
        return true;
    } 

    /*Some input has value c */
    // ^^^ is wrong in 9Valued one ! 
   // if (curGate->Evaluate() != U) 
    if ( !isNotKnown( curGate->Evaluate() ) )
    {

        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
        return false;
    }


    if (inputUCount>1) 
    {
        //(*iter)->value = curValue; <-- DOUBT: ?
        // if it is the faulty wire, don't set the value
        if ( curWire != (circuit.faultWire) )
        {
            cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
        
            Intentions.push_back(curImplication);
        
        }    
        cout<<__FILE__<<__LINE__ << "    " << "Adding to J Frontier:  " << (*iter)->id << endl;
        Add_To_JFrontier((*iter),curValue);
        (ImpliQueue).pop();
        return true;
    }

    if (inputUCount == 1)
    {
        for (iter = (curGate->inputs).begin();iter != (curGate->inputs).end();iter++)
        {
            if (Check_Wire_Value_For_Assignment((*iter),U))
            {

                Value newVal = (ControlValues[curGate->gtype]);
                Implication* newImply= new Implication(*iter,newVal,false); /*bool false = 0 = backward*/
                cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << (*iter)->id 
                    << "value:  " <<  newVal << endl;
                (ImpliQueue).push(newImply); 

            }
        }

        (ImpliQueue).pop();
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
    {     
      cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
           
        Intentions.push_back(curImplication);
	
    }    
    return true;

    }

}


bool  ATPG::Check_Wire_Value_For_Assignment(Wire* wire,Value val)
{
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
                cout<<__FILE__<<__LINE__ << "    " <<"Contradiction:  prev." <<  (*iter)->wire->value << " new: " << val << endl;
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
    cout<<__FILE__<<__LINE__ << "    " << "Value to be implied" << curValue << endl;
    cout<<__FILE__<<__LINE__ << "    " << "Value by Xor" << Do_Xor((Value)((~ControlValues[curGate->gtype])&0xf),(Value)InversionValues[curGate->gtype])  << endl;
    assert(curValue == Do_Xor((Value)((~ControlValues[curGate->gtype])&0xf),(Value)InversionValues[curGate->gtype]));
    assert(curGate);


    /* Check if the gate output value is either unknown
     * or what we want i.e. cbar xor i
     */

    /* This implication resolved if wire is already what we want */
    if (curWire->value == curValue) 
    {
        cout<<__FILE__<<__LINE__ << "    " << "Curvalue is what we want. Implication resolved by itself"
            << endl;
        (ImpliQueue).pop();
        // At this point, justifying the inputs may be pending
        // hence we dont return here
    } 

/*    if (curWire->value != U) 
    {
        cout<<__FILE__<<__LINE__ << "    " << "curWire->value is : " << curWire->value << endl;
        cout<<__FILE__<<__LINE__ << "    " << "Returning false" << endl;
        return false;
    }
*/

    /*Check if all inputs of the gate are unknown or cbar*/
    /*TODO:You might want to iterate through the input list
     * instead. 
     */


    if (curGate->Evaluate() == curValue)
    {
        cout<<__FILE__<<__LINE__ << "    " << "Gate evals to what we want. Implication to be resolved "
            << endl;
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
    {
        cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
        
        Intentions.push_back(curImplication);
    }   
    
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
                    Value newVal = (Value)( (~(ControlValues[curGate->gtype])) & 0xf);
                    Implication* newImply= new Implication(*iter,newVal,false); /*bool false = 0 = backward*/
                    cout<<__FILE__<<__LINE__ << "    " << "Adding implication :  " << (*iter)->id 
                        << "value:  " <<  newVal << endl;
                    (ImpliQueue).push(newImply); 
                }
		// if it is the faulty wire, don't set the value
		if ( curWire != (circuit.faultWire) )
        {
            cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
        
            Intentions.push_back(curImplication);
	    }
        (ImpliQueue).pop();
                return true;
            }


        default:
            /*Some input has value c */
            {

                cout<<__FILE__<<__LINE__ << "    " << "Evaluate of " << curGate->id << " gives:" << curGate->Evaluate() << endl;
                cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << endl ;
                cout<<__FILE__<<__LINE__ << "    " << "Returning false" << endl;
                return false;
            }
    }

}




bool ATPG::Add_To_JFrontier(Wire *wire,Value value)
{
    circuit.JFrontier.push_back(WireValuePair(wire,value));
    ATPGPRINT(ATPG_DFILE,"Added a gate to JFrontier. JFrontier is now:");
    cout<<__FILE__<<__LINE__ << "    " << "Added a gate to JFrontier." << endl;
    PRINTJFRONTIER;
    return true;
    
}
bool ATPG::Add_To_DFrontier(Wire *wire,Value value)
{
    //circuit.DFrontier.push_back(WireValuePair(wire,value));
    circuit.DFrontier.push_front(WireValuePair(wire,value));
    ATPGPRINT(ATPG_DFILE,"Added a gate to DFrontier. DFrontier is now:");
    PRINTDFRONTIER;
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
    ATPGPRINT(ATPG_DFILE,"Removed a gate from DFrontier. DFrontier is now:");
    PRINTDFRONTIER;
  
    return result;
}



bool ATPG::RemoveFromJ(Wire *wire)
{
    cout<<__FILE__<<__LINE__ << "    " << "Kashyap: Called remove from J" << endl;
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
    ATPGPRINT(ATPG_DFILE,"Removed a gate from JFrontier. JFrontier is now:");
    PRINTJFRONTIER;
  
    return result;
}



bool ATPG::Imply_And_Check()
{
    cout<<__FILE__<<__LINE__ << "    " << "Imply_And_Check called" << endl;
    
    assert(Intentions.empty());



    while (!ImpliQueue.empty())
    {

        //  Step 1: Get Details of current implication 
        Implication*  curImplication = (ImpliQueue).front();
        Wire* curWire = curImplication->wire;
        Value curValue = curImplication->value;
        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": ";
        cout<<__FILE__<<__LINE__ << "    " <<"New implication from the queue:  " << (curWire->id) <<  ":   "<< (curValue) ;
        if (curImplication->direction == 0) cout<<__FILE__<<__LINE__ << "    " <<   "  backwards" << endl;
        else cout<<__FILE__<<__LINE__ << "    " << "  forward" << endl;
            
        //  Resolving backward implication
        if (curImplication->direction == false) 
        {
            if (Resolve_Backward_Implication(curImplication,curWire,curValue) == false)
            {

                cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
                goto fail;
            }
                else continue;

        }
        //  Resolving forward implication
        else
        {
            if (Resolve_Forward_Implication(curImplication,curWire,curValue) == false)
         {

                cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
                goto fail;
         
         }
                else continue;

        }
    }

    Make_Assignments();
    return true;

fail:
    Failure();
    return false;
}


void ATPG::Make_Assignments()
{
    assert(ImpliQueue.empty());
   list<Implication*>::iterator iter= Intentions.begin();
   for (; iter!=Intentions.end();iter++)
    {
        // Intended value is on the RHS
        (*iter)->wire->value = (*iter)->value;
    }
   Intentions.clear();
}



void ATPG::Failure()
{
    cout << " Ha Ha Ha ! I am failing :) " << endl;
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

        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
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

        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
        return false;
    }

    PRINTDFRONTIER;
    //for (;checkIter !=(circuit.DFrontier).end();checkIter++)
    // Remember a stack and understand how this works
    // Add to D frontier function, now always pushes into 
    // this stack. And we choose and set the implications, which will pop off the 
    // top one. A few more will be added on the top.
    while ( !circuit.DFrontier.empty() )
    {
	checkIter = (circuit.DFrontier).begin();
        /*Selecting a gate from the DFrontier*/
	cout << __LINE__ << "Current size of D = " << circuit.DFrontier.size() <<  "About to take " << checkIter->iwire->id << "  from D" << endl;
       Gate* curGate=dynamic_cast<Gate*>(checkIter->iwire->input);
       Value cval = ControlValues[curGate->gtype];
        
      list<Wire*>::iterator inputIter = (curGate->inputs).begin();
      for (;inputIter != (curGate->inputs).end();inputIter++)
      {
          if ((*inputIter)->value == U)
            {
                
                Implication* newImply= new Implication(*inputIter,(Value)((~cval)&0xf),false); /*bool false = 0 = backward*/
                cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    "  << ":  " ;
                cout<<__FILE__<<__LINE__ << "    " << "Adding backward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
                newImply= new Implication(*inputIter,(Value)((~cval)&0xf),true); /*bool false = 0 = backward*/
                cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    "  << ":  " ;
                cout<<__FILE__<<__LINE__ << "    " << "Adding forward implication: " << (*inputIter)->id
                     <<  ~(cval) << endl; 
                (ImpliQueue).push(newImply);
            }         
          else
              cout<<__FILE__<<__LINE__ << "    " <<  "DEBUG: Wire: " << (*inputIter)->id << "value: " <<  (*inputIter)->value <<endl; 


      }

      /* If the D-Drive succeeded through this gate, well and good, return true
       * Else we try the next gate in the D-frontier
       */
     if (D_Algo() == true) return true;
    }

JFrontierWork:
    if (circuit.JFrontier.empty()) return true;
    checkIter = (circuit.JFrontier).begin();

    PRINTJFRONTIER;
    
    //  Selecting a gate from the JFrontier
    //  A note about the J Frontier:
    //  We add a gate to the J Frontier if:
    //  1. Its output is c (control val) xor i
    //  2. more than input has value x
    //  3. none of the inputs which currently have
    //  known values have value c
    for (;checkIter !=(circuit.JFrontier).end();checkIter++)
    {
       Gate* curGate=dynamic_cast<Gate*>(checkIter->iwire->input);
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


bool ATPG::Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
{
    assert(curImplication->direction ==true);

    // if the value of the wire is already set
    // And if it is the right value or not
    if (!Compatible(curWire->value,curValue))
    {
        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Returning false" << endl;
        return false;
    }

    // if it is a PO, set the value to the wire 
    // and pop off the implication 
    if ((curWire->outputs).empty())
    {
        // set the value of the current wire
        //curWire->value = curValue;
        //dont set the value, add an intention
        Intentions.push_back(curImplication);
         cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " <<  curImplication->value << endl; 
        (ImpliQueue).pop();
        cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ": Success, po reached and po value set to:" << curValue << endl;
        return true;
    }


    Element *curEle = *((curWire->outputs).begin());

    // Handle the fanout case - forward implication
       if ( curEle->type ==  WIRE )
       {
	       // first set the value of the current wire
	       cout << __LINE__ << ":FANOUT HANDLING IN FORWARD IMPLI" << endl;
	       Intentions.push_back(curImplication);
	       list<Element*>::iterator iter=(curWire->outputs).begin();
	       for (; iter != (curWire->outputs).end(); iter++)
	       {
		       Wire *owire = dynamic_cast<Wire *> (*iter);
		       assert(owire != NULL);
		       Implication* newImply = new Implication(owire, curImplication->value,true); /*bool true = 0 = forward*/
		       (ImpliQueue).push(newImply); 
	       }
	       // pop off the current implication and return
	       (ImpliQueue).pop();
	       return true;
       }

    Gate *curGate = dynamic_cast<Gate*>(curEle);
    assert(curGate);
    
    Value gateOldOutput = curGate->output->value;
    Value wireOldValue = curWire->value;

    // set the value for the wire, ie curValue to wire->value
    Intentions.push_back(curImplication);
     cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 
            <<  curImplication->value << endl; 
        
    Value gateNewOutput = curGate->Evaluate();

    // if the old gate output is not unknown and 
    // the new value doesn't agree with old 
    // ( eg: 1 and u/0 don't agree, but D and u/0 or D and 1/u agree )
    //if ( (~(gateOldOutput & U)) && (gateNewOutput & geteOldOutput) )
    if ((curGate->output != circuit.faultWire) && !Compatible(gateOldOutput,gateNewOutput))
   // if ( (gateOldOutput != U) && (gateNewOutput != U)  
     //       && ( ((gateNewOutput&gateOldOutput) != gateNewOutput) 
       //         ||   ((gateNewOutput|gateOldOutput) != gateOldOutput) ))
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
	    if ( (curValue == D) || (curValue == DBAR))
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

        if (RemoveFromD(curGate->output))
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

        if (RemoveFromJ(curGate->output))
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






bool ATPG::Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
{

    // Check if the wire is a PI.
    // If yes just set the value and
    // we are done with the implication 
    if ((curWire->input) == NULL)
    {
        cout<<__FILE__<<__LINE__ << "    " << "Reached PI." << endl;
         cout<<__FILE__<<__LINE__ << "    " << "New intention:  " <<  curImplication->wire->id << "  " 	<<  curImplication->value << endl; 
        ImpliQueue.pop();
	if (curWire != circuit.faultWire) 	// Did we forget this ?
            Intentions.push_back(curImplication);
        return true;
    }

    // Handle two cases. Fanout and no fanout. Right now,
    // Each wire has only a gate for an input.
    if ((curWire->input)->type == WIRE)
    {
	    cout << __LINE__ << " Handline fanout from backward implication" << endl;
	    // first set the value of the current wire
	    if (curWire != circuit.faultWire )
	        Intentions.push_back(curImplication);

	    Wire *stemWire = dynamic_cast<Wire *> (curWire->input);
	    // add a new implication for the stem - backward
	    Implication* newImply = new Implication(stemWire, curImplication->value,false); /*bool false= 1 = backward*/
	    (ImpliQueue).push(newImply); 

	    // add implications for the remaining branches of the input
	    list<Element*>::iterator iter=(stemWire->outputs).begin();
	    for (; iter != (stemWire->outputs).end(); iter++)
	    {
		    Wire *owire = dynamic_cast<Wire *> (*iter);
		    // if this assert fails == resolve branches failure
		    assert(owire != NULL);
		    if ( owire != curWire )
		    {
			 // add forward implications for the remaining branches, other than the current one
		    	 newImply = new Implication(owire, curImplication->value,true); /*bool true = 0 = forward*/
		    	(ImpliQueue).push(newImply); 
			cout << __LINE__ << " added this wire to the impli queue: " << owire->id << endl;
		    }
	    }

	    // pop off the current implication and return
	    ImpliQueue.pop();
	    return true;
    }

    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert(curGate);
    
    cout<<__FILE__<<__LINE__ << "    " << "Backward implication is on this gate's output: " << curGate->id << endl;
 //   cout<<__FILE__<<__LINE__ << "    " << "c xor i is  "<<  Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]) <<  endl;
   // cout<<__FILE__<<__LINE__ << "    " << "Value to be implied is : " <<  curValue << endl;


    cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ":   " << curValue << endl;
    cout<<__FILE__<<__LINE__ << "    " << __LINE__ << "    " << ":   " << Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype])  << endl;

    if ( curValue == Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype]))
    {
        return Handle_Output_Coming_From_Control_Value(curImplication,curWire,curValue);
    }
    else if  ( curValue == Do_Xor((Value)((~ControlValues[curGate->gtype])&0xf),InversionValues[curGate->gtype]))
 
    {
       return Handle_Output_Coming_From_Noncontrol_Value(curImplication,curWire,curValue);
    }
    else 
    {
        cout<<__FILE__<<__LINE__ << "    " <<"Something wrong"<< endl;
        assert(false);
    }
}



Implication* Find_In_Intentions_List(Wire* wire)
{
    list<Implication*>:: iterator iter = Intentions.begin();
    for (;iter!=Intentions.end();iter++)
    {
         if (wire == (*iter)->wire) 
            return (*iter);
    }
    return NULL;
}




#include "circuit.h"
#include <queue>
using namespace std;


static int cktDebug=1;
#define ERROR(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n");exit(0);}  else exit(0);}
#define DEBUG(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n")}


Value ControlValues[7] = {ZERO,ONE,U,ZERO,ONE,U,U};

Value InversionValues[7] = {ZERO,ZERO,U,ONE,ONE,U,U};





void Circuit::Add_Gate_To_Wire_Output(Gate* gate,const char* wirename)
{
    Wire* iwire = ((circuit.Netlist).find(wirename))->second;
    assert( (circuit.Netlist).find(wirename) != (circuit.Netlist).end());
 //   cout << "Added gate  "  << gate->id <<  "  as output of wire  " << iwire->id << endl;
    iwire->outputs.push_back((Element*)gate);    
    return;
}


void Circuit::Add_Gate_To_Wire_Input(Gate* gate,const char* wirename)
{
    Wire* iwire = ((circuit.Netlist).find(wirename))->second;
    assert( (circuit.Netlist).find(wirename) != (circuit.Netlist).end());
  //  cout << "Added gate  "  << gate->id <<  "  as input of wire  " << iwire->id << endl;
    iwire->input = (Element*)gate;    
    return;
}



/*
 * Circuit::methods
 */
bool  Circuit::AddWire(const char *inName,WireType type)
{
	Wire *iwire = new Wire(inName,type);
	Netlist.insert( pair<string,Wire *>(inName,iwire) );
    iwire->value = U;
	
    if (type == PI)
        PriInputs.insert( pair<string,Wire*>(inName,iwire) );
	else if (type == PO)
		PriOutputs.insert( pair<string,Wire*>(inName,iwire) );

	return true;
}



/* By the time a gate is added, all the wires have been added and 
 * their types are known
 * */

bool Circuit::AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
    Gate *gate = new Gate(name,type);
    map<string,Wire *>::iterator iter;

    // Some consistency checks before we add
    // the wires to the gates inputs and output
    // 1. Output must be a valid wire
    iter = Netlist.find(output);
    if (iter  ==  Netlist.end()) 
    {
        ERROR("Gate's output name %s does not represent a valid wire",output);
        return false;
    }

    // 2. If output type is PO, it should be
    // there in PO list
    if ((iter->second)->wtype == PO)
    {
        iter = PriOutputs.find(output);
        if (iter  ==  PriOutputs.end()) 
        {
            ERROR("Gate's output name %s is supposedly PO but not present in PO list",output);
            return false;
        }
    }

    // Now that we are convinced, add the wire as gate's output
    // and the gate as wire's input :P
    Add_Gate_To_Wire_Input(gate,((iter->second)->id).c_str());
    gate->output = iter->second;

    while (numSignals--)
    {


        // Some consistency checks before we add
        // the wires to the gates inputs and output
        // 1. Input must be a valid wire
        iter = Netlist.find(inputs[numSignals]);
        if (iter  ==  Netlist.end()) 
        {
            ERROR("Gate's input name %s does not represent a valid wire",inputs[numSignals]);
            return false;
        }

        // 2. If input type is PI, it should be
        // there in PI list
        if ((iter->second)->wtype == PI)
        {
            iter = PriInputs.find(inputs[numSignals]);
            if (iter  ==  PriInputs.end()) 
            {
                ERROR("Gate's input name %s is supposedly PI but not present in PI list", inputs[numSignals]);
                return false;
            }
        }

        // Now that we are convinced, add the wire as gate's input
        gate->inputs.push_back(iter->second);
        Add_Gate_To_Wire_Output(gate,((iter->second)->id).c_str());
    }

    // Finally we add the gate to the circuit list
    gate->tempInputs = (gate->inputs).size();
    circuit.Gates.insert( pair<string,Gate *>(name, gate) );

    return true;
}



bool Circuit::Evaluate()
{
    int level=1;
    multimap<int,Element*>:: iterator levelIter;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();


    // Adding gen. values to the primary inputs 
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        (iter->second)->value=ZERO;
    }


    do
    {
        /* first and second are the end-point iterators that has been
         * returned on qeurying the multimap on a value
         */
        for (levelIter = ((circuit.Levels).equal_range(level)).first; 
                levelIter !=((circuit.Levels).equal_range(level)).second; levelIter++)
        {
            //   cout <<"Iter is now at" << levelIter->second->id  <<endl;
            if ( (levelIter->second)->type == GATE)
            {
                Gate* curGate = dynamic_cast<Gate*>(levelIter->second);
                Value curValue = curGate->Evaluate();
                Wire* outputWire = curGate->output;
                outputWire->value = curValue;
                if  ((outputWire->outputs).size() > 1) 
                {
                    list<Element*>::iterator iter = (outputWire->outputs).begin();
                    cout << "printing output list of wire: " << outputWire->id<< endl;

                    /* Reasons for the dynamic cast: 
                     * If the output wire has > 1 output, they are all bound to
                     * be wires.
                     * */
                    for (;iter != (outputWire->outputs).end(); iter++)
                    {
                        cout << "output: " << (*iter)->id << endl;
                        Wire* check = (dynamic_cast<Wire*>(*iter));
                        assert(check);
                        check->value = curValue;

                    }

                }



            }
            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 

    iter = (circuit.PriOutputs).begin();
    
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        cout << "PO: " << (iter->second)->id << "value:  " << (iter->second)->value << endl;
    }

    return true;
}


bool Circuit::Levelize()
{
    int level=0;
    map<string,Wire*>::iterator iter =  (circuit.PriInputs).begin();
    multimap<int,Element*>:: iterator levelIter;


    // Adding the primary inputs to level zero
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        cout << "Adding  " << iter->second->id << "at level 0" << endl;
        (circuit.Levels).insert(pair<int,Element*>(0,(Element*)iter->second));
    }

    do
    {
        for (levelIter = ((circuit.Levels).equal_range(level)).first; 
                levelIter !=((circuit.Levels).equal_range(level)).second; levelIter++)
        {
         //   cout <<"Iter is now at" << levelIter->second->id  <<endl;
            if ( (levelIter->second)->type == GATE)
            {
                Gate* curGate = dynamic_cast<Gate*>(levelIter->second);
                Wire* curWire = curGate->output;
                
               // cout << "Found gate" << curGate->id << "for search at level " << level<< endl; 
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        cout << "Adding  " << curGate->id << "at level" << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }
            }
            else if  ( (dynamic_cast<Wire*>(levelIter->second))->wtype == PI )
            {
                Wire* curWire =   dynamic_cast<Wire*>(levelIter->second);
             //   cout << "Found PI" << curWire->id << "for search at level " << level<< endl; 
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        cout << "Adding  " << curGate->id << "at level" << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }

            } 

            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 
    cout << "Levelization completed" << endl;
}



void Circuit:: ResolveWire(Wire* wire)
{
    // Assuming that initially all outputs of
    // a wire are gates
    //Steps:
    // 1. Find out if a wire has more than one outputs
    // 2. Create a new wire - with input as the orig wire
    // and output as the gate
    // 3. Update the orig wire's outputs to reflect the wires rather than gates
    // 4. Update the output gates (orig gates) inputs showing the new wires
    // as inputs

    cout << "Need to resolve wire:   " << wire->id << endl;
    list<Element*>:: iterator iter = (wire->outputs).begin();
    while ( iter != (wire->outputs).end() )
    {
        Element* curEle = (*iter);
        if (curEle->type == GATE)
        {

        Gate* gate = dynamic_cast<Gate*>(*iter);
        assert(gate); // dynamic_cast must not fail
        string newname = (wire->id)+"_"+(gate->id);
        newname = Check_Name_Present(newname);
        // step 2. Add a new wire for this instance
        cout << "Adding derived wire:    " <<  newname  << endl;
        circuit.AddWire(newname.c_str(),CONNECTION);

        // The new wire's output should be the 
        // gate
        Add_Gate_To_Wire_Output(gate,newname.c_str());

        // The new wire's input should be the old wire and
        // the old wire' output should change from gate to 
        // new wire

        Wire* newwire = ((circuit.Netlist).find(newname))->second;
        newwire->input = wire;
         // finally update the gate's inputlist
        (gate->inputs).remove(wire);
        (gate->inputs).push_back(newwire);
        
        (*iter) = newwire;

       iter++;
        }


        else continue;
    }

    if (wire->wtype == PO)
    {
        cout << "***************PO with fanout detected*************" <<  endl;
        assert(false);
    }
}





bool Circuit::ResolveBranches()
{
   map<string,Wire*>:: iterator iter = (circuit.Netlist).begin();
    while (iter != (circuit.Netlist).end())
    {
        Wire* iwire = iter->second;
        
        if (  ( (int)(iwire->outputs).size() > 1 
            || ( (int) (iwire->outputs).size() > 0  && iwire->wtype == PO)) 
            && (Wire_Not_Derived(iwire)))
                ResolveWire(iwire);
        iter++;
    }
    cout << "Resolve branches completed successfully" << endl;
    return true;
}


string Circuit::intToString(int inInt)
{
    stringstream ss;
    string s;
    ss << inInt;
    s = ss.str();
    return s;
}

string Circuit::Check_Name_Present(string givenname)
{
    map<string,int>::iterator iter = (circuit.RepeatInputs).find(givenname);
    cout << givenname <<endl;
    if (iter != (circuit.RepeatInputs).end())
    {
            iter->second += 1;
            string newname;
            newname = givenname + "_" + intToString(iter->second);
            cout << "Returning name   " << newname << endl;
         //   cout << "int value is  "  << iter->second << endl;
            return newname;
    }
    else 
     {
         (circuit.RepeatInputs).insert(pair<string,int>(givenname,0));
         cout << "Adding the namemap" << givenname << endl;
        //    cout << "Returning name   " << givenname << endl;
         return  givenname;
     }

}


bool Circuit::Wire_Not_Derived(Wire* wire)
{
    if (strstr((wire->id).c_str(),"_")) return false;
    else return true;
}

/*Use only to check if value is c xor i or  cbar xor i*/
bool Do_Xor(bool val1, bool val2)
{

    bool output;
    output = (val1 & (~(val2)) | ((~val1) & (val2)));
    return output&(0xf);
}


int Translate_Value_To_Int(Value value)
{
    switch(value)
    {
        case 0x0: 
            return 0;
        case 0xff:
            return 1;
    }
    return 0;
}


bool Handle_Output_Coming_From_Noncontrol_Value(Implication* curImplication, Wire* curWire,Value curValue)
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
        (circuit.ImpliQueue).pop();
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
        (circuit.ImpliQueue).pop();
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
                    (circuit.ImpliQueue).push(newImply); 
                }
		// if it is the faulty wire, don't set the value
		if ( curWire != (circuit.faultWire) )
		    curWire->value = curValue;
		(circuit.ImpliQueue).pop();
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


// Total mess !! plz decorate it :)
bool Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value curValue)
{
    Gate* curGate = dynamic_cast<Gate*>(curWire->input);
    assert(Translate_Value_To_Int
            (curValue) == (Do_Xor(ControlValues[curGate->gtype],InversionValues[curGate->gtype])));

    list<Wire*>::iterator iter = (curGate->inputs).begin();
    int inputUCount = 0;

    for (;iter != (curGate->inputs).end();iter++)
    {
        if ((*iter)->value == U) inputUCount++;
    }


    // TODO: The we need to propagate further
    if (curWire->value == curValue) 
    {
        cout << "Curvalue is what we want. Implication resolved by itself" << endl;
        (circuit.ImpliQueue).pop();
        return true;
    } 

    if (curWire->value != U) 
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
	     curWire->value = curValue;
        (circuit.ImpliQueue).pop();
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
	     curWire->value = curValue;
        cout << "Adding to J Frontier:  " << (*iter)->id << endl;
        circuit.Add_To_JFrontier((*iter),curValue);
        (circuit.ImpliQueue).pop();
        return true;
    }

    if (inputUCount == 1)
    {
        for (iter = (curGate->inputs).begin();iter != (curGate->inputs).end();iter++)
        {
            if ((*iter)->value == U)
            {

                Value newVal = (ControlValues[curGate->gtype]);
                Implication* newImply= new Implication(*iter,newVal,false); /*bool false = 0 = backward*/
                cout << "Adding implication :  " << (*iter)->id 
                    << "value:  " <<  newVal << endl;
                (circuit.ImpliQueue).push(newImply); 

            }
        }

        (circuit.ImpliQueue).pop();
	// if it is the faulty wire, don't set the value
	if ( curWire != (circuit.faultWire) )
	     curWire->value = curValue;
	return true;

    }

}




bool Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
{

    // Check if the wire is a PI.
    // If yes just set the value and
    // we are done with the implication 
    if ((curWire->input) == NULL)
    {
        cout << "Reached PI." << endl;
        curWire->value = curValue;
        (circuit.ImpliQueue).pop();
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



bool Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value curValue)
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
        (circuit.ImpliQueue).pop();
        cout << __LINE__ << ": Success, po reached and po value set to:" << curValue << endl;
        return true;
    }



    // Handle the fanout case
    /*
       if ( curEle->type ==  WIRE )
       {
       Wire* iwire = dynamic_cast<Wire*> curEle;
       (circuit.ImpliQueue).push(newImply); 
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
            circuit.Add_To_DFrontier(curGate->output, gateNewOutput);
            // Set the value of the value (useful in 9V)
            // curGate->output->value = gateNewOutput;
            // pop off the implication and proceed !
            circuit.ImpliQueue.pop();
            return true;
        }

        // Now just propage the value further
        // But before that check if the gate is in D frontier 
        // and remove the gate from it. Because it is resolved now

        if (circuit.RemoveFromD(curGate->output))
           ;
            // cout << "INFO: The gate is indeed in D and has been removed" << curGate->id << endl;
        else 
            ;
            //cout << "INFO: The gate is not there in D frontier. report from " << __LINE__ << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (circuit.ImpliQueue).pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        cout << "Adding implication :  " << curGate->output->id << ". Line: " << __LINE__ << endl;
        (circuit.ImpliQueue).push(newImply); 

	return true;

    }

    // This is the case where the gate ouput is previously known.
    // Reason: Due to some implication, which finally added this 
    // gate to J frontier  
    else 
    {
        if ( gateNewOutput == U )
        {
            // pop off the current impli
            (circuit.ImpliQueue).pop();
            return true;
        }

        if (circuit.RemoveFromJ(curGate->output))
            cout << "INFO: The gate is indeed in J and has been removed" << curGate->id << endl;
        else 
            cout << "INFO: The gate is not there in J frontier. report from " << __LINE__ << endl;

        // The last thing to do is to propagate the impli and
        // before that, popping off the current impli

        // pop off the current impli
        (circuit.ImpliQueue).pop();
        Implication* newImply= new Implication(curGate->output,gateNewOutput,true); /*bool true = 0 = forward*/
        cout << "Adding implication :  " << curGate->id << ". Line: " << __LINE__ << endl;
        (circuit.ImpliQueue).push(newImply); 
        return true;
    }

    cout << __LINE__ << ": Returning false" << endl;
    return false;

}





bool Circuit::Imply_And_Check()
{
    cout << "Imply_And_Check called" << endl;
 
    while (!circuit.ImpliQueue.empty())
    {

        //  Step 1: Get Details of current implication 
        Implication*  curImplication = (circuit.ImpliQueue).front();
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
    return true;
}


bool Circuit::D_Algo()
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




bool Circuit::Add_To_JFrontier(Wire *wire,Value value)
{
    JFrontier.push_back(WireValuePair(wire,value));
    return true;
    
}
bool Circuit::Add_To_DFrontier(Wire *wire,Value value)
{
    DFrontier.push_back(WireValuePair(wire,value));
    return true;
    
}



bool Circuit::RemoveFromD(Wire *wire)
{
    bool result=false;
    list<WireValuePair>::iterator iter = DFrontier.begin();

    while (iter != DFrontier.end())
    {
        if ( (iter->iwire) == wire )
        {
            iter = DFrontier.erase(iter);
            result = true;
            continue;   // we can return from function
        }
        iter ++;
    }

    return result;
}



bool Circuit::RemoveFromJ(Wire *wire)
{
    bool result=false;
    list<WireValuePair>::iterator iter = JFrontier.begin();

    while (iter != JFrontier.end())
    {
        if ( (iter->iwire) == wire )
        {
            iter = JFrontier.erase(iter);
            result = true;
            continue;   // we can return from function
        }
        iter ++;
    }

    return result;
}

// returns if the value is known or not
// 
bool isNotKnown(Value v)
{
    switch(v)
     {
	    case ONE:
	    case ZERO:
	    case D:
	    case DBAR:
	    return false;
	
	default:
	    return true;
     }
     return true;
}


    
/*
 * Gate Evaluation  
 * We use a table of function pointers.
 * The appropriate function is invoked by indexing 
 * into the table using the gatetype field of the calling
 * gate object
 */

Value Gate::Evaluate()
{
    return g_EvaluateTable[gtype](inputs);
}





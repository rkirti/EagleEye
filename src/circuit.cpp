#include "circuit.h"
#include <queue>
using namespace std;


static int cktDebug=1;
#define ERROR(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n");exit(0);}  else exit(0);}
#define DEBUG(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n")}




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
    if (wire->id.find("_")) return false;
    else return true;
}

/*Use only to check if value is c xor i or  cbar xor i*/
Value Do_Xor(Value val1, Value val2)
{

    Value output;
    Value negval1 =  (Value) (val1 != U)?(Value)(~(val1)&0xf):U;
    Value negval2 =  (Value) (val2 != U)?(Value)(~(val2)&0xf):U;
    output = (Value)((( (val1 & negval2) | (negval1 & val2)))&0xf);
    return output;
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





void Circuit::Print_All_Wires()
{

    // Print out the primary inputs
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    cout<< "Test vectors are : " << endl;
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        cout<< (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the primary outputs
    iter =  (circuit.PriOutputs).begin();
    cout<< "Output vectors are : " << endl;
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        cout<< (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the netlist
    iter =  (circuit.Netlist).begin();
    cout<< "Netlist : " << endl;
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        cout<< (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


}


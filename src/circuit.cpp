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
    cout << "Added gate  "  << gate->id <<  "  as output of wire  " << iwire->id << endl;
    iwire->outputs.push_back((Element*)gate);    
    return;
}


void Circuit::Add_Gate_To_Wire_Input(Gate* gate,const char* wirename)
{
    Wire* iwire = ((circuit.Netlist).find(wirename))->second;
    assert( (circuit.Netlist).find(wirename) != (circuit.Netlist).end());
    cout << "Added gate  "  << gate->id <<  "  as input of wire  " << iwire->id << endl;
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
    circuit.Gates.insert( pair<string,Gate *>(name, gate) );

    return true;
}

class WireAndLevel 
{
    public:
	Wire *wire;
	int level;
	WireAndLevel(Wire *givenWire, int Level)
	{
	    wire = givenWire;
	    level = Level;
	}
};

bool Circuit::Levelize()
{

    queue<WireAndLevel> myqueue;

    map<string,Wire *>::iterator iter;
    for (iter=circuit.PriInputs.begin(); iter != circuit.PriInputs.end(); iter++)
	myqueue.push (WireAndLevel(iter->second,0));

    while ( !myqueue.empty() )
    {
	WireAndLevel temp=myqueue.front();
	//cout << temp.wire->id << " and clevel=" << temp.level << endl;
	list<Element *>::iterator listIter = ((temp.wire)->outputs).begin();
	for ( ; listIter != ( (temp.wire)->outputs).end(); listIter++)
	{
	    if ( (*listIter)->type != GATE )
	    {
		cout << "error ! you should call levelize before resolving brances :)" << endl;
		return false;
	    }
	    Gate *gate = dynamic_cast<Gate *>(*listIter);
	    gate->level++;	// used temporarily :)
	    //cout << "gate level=" << gate->level << "gate inputs="<< gate->inputs.size() << "name=" << gate->id << endl;
	    if ( gate->level >= gate->inputs.size() )
		gate->level = temp.level;	// freeze the level and remember that this will not happen again. Prove it !

	    // Now push the gate's output into the queue with current level++
	    myqueue.push (WireAndLevel(gate->output, temp.level+1));
	}
	myqueue.pop();
    }

    return true;
}


    
/*
 * Gate::methods
 */
Value Gate::Evaluate()
{
    return g_EvaluateTable[gtype](inputs);
}

/*
 * The logic functions - AND, OR, NOT, NAND, NOR
 */
static Value And(list<Wire*> inputs)
{
   int output=ONE;	// initially the ouput should be 1, so that it outputs sets to the starting value on doing the first and
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output &= (*iter)->value;
    return (Value)output;

}


static Value Or(list<Wire*> inputs)
{
   int output=ZERO;	// initially the ouput should be 0, so that it outputs sets to the starting value on doing the first or 
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output |= (*iter)->value;
    return (Value)output;

}

static Value Not(list<Wire*> inputs)
{
   list<Wire *>::iterator iter;
   iter=inputs.begin();
   return (Value)(~((*iter)->value));
}

static Value Nand(list<Wire*> inputs)
{
   int output=ONE;	// initially the ouput should be 1, so that it outputs sets to the starting value on doing the first and
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output &= (*iter)->value;
    return (Value)((~output)&0xf);

}


static Value Nor(list<Wire*> inputs)
{
   int output=ZERO;	// initially the ouput should be 0, so that it outputs sets to the starting value on doing the first or 
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output |= (*iter)->value;
    return (Value)((~output)&0xf);

}

static Value Xor(list<Wire *> inputs)
{
	int output=ZERO;
	list<Wire *>::iterator iter;
	for (iter=inputs.begin(); iter != inputs.end(); iter++)
		output = ((output & (~(*iter)->value)) | ((~output) & ((*iter)->value)));
	return (Value)output;
}


const GateEvaluate g_EvaluateTable[] = 
{
    And,
    Or,
    Not,
    Nand,
    Nor,
    Xor
};


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

    cout << "Need to resolve wire" << wire->id << endl;
    list<Element*>:: iterator iter = (wire->outputs).begin();
    while ( iter != (wire->outputs).end() )
    {
        Element* curEle = (*iter);
        if (curEle->type == GATE)
        {

        Gate* gate = dynamic_cast<Gate*>(*iter);
        assert(gate); // dynamic_cast must not fail
        string newname = (wire->id)+"_"+(gate->id);
        // step 2. Add a new wire for this instance
        circuit.AddWire(newname.c_str(),CONNECTION);

        // The new wire's output should be the 
        // gate
        Add_Gate_To_Wire_Output(gate,newname.c_str());

        // The new wire's input should be the old wire and
        // the old wire' output should change from gate to 
        // new wire

        Wire* newwire = ((circuit.Netlist).find(newname))->second;
        newwire->input = wire;
        
        (*iter) = newwire;

        // finally update the gate's inputlist
        (gate->inputs).remove(wire);
        (gate->inputs).push_back(newwire);
        iter++;
        }


        else continue;
    }

    if (wire->wtype == PO)
    {
        cout << "***************PO with fanout detected*************" <<  endl;
    }
}





bool Circuit::ResolveBranches()
{
   map<string,Wire*>:: iterator iter = (circuit.Netlist).begin();
    while (iter != (circuit.Netlist).end())
    {
        Wire* iwire = iter->second;
        
        if ( ((int)(iwire->outputs).size() > 1) || ( ( (int) (iwire->outputs).size() > 0 ) && iwire->wtype == PO ))
                ResolveWire(iwire);
        iter++;
    }
    return true;
}

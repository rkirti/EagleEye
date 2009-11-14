#include "circuit.h"
using namespace std;

/*
 * Circuit::methods
 */
bool  Circuit::AddWire(char *inName,WireType type)
{
	Wire *iwire = new Wire(inName,type);
	Netlist.insert( pair<string,Wire *>(inName,iwire) );

	if (type == PI)
		PriInputs.insert( pair<string,Wire *>(inName,iwire) );

	else if (type == PO)
		PriOutputs.insert( pair<string,Wire *>(inName,iwire) );

	return true;
}

bool Circuit::AddGate(GateType type, char *name,char* output,char **inputs,int numSignals)
{
	Gate *gate = new Gate(name,type);
	map<string,Wire *>::iterator iter;

	iter = PriOutputs.find(output);
	if (iter == PriOutputs.end())
		return false;
	
	gate->output = (*iter).second;

	while (numSignals--)
	{
		iter = PriInputs.find(inputs[numSignals]);
		if (iter == PriInputs.end())
			return false;

		gate->inputs.push_back((*iter).second);
		
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
   int output=U;
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output &= (*iter)->value;
    return (Value)output;

}


static Value Or(list<Wire*> inputs)
{
   int output=U;
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
   int output=U;
   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output &= (*iter)->value;
    return (Value)((~output)&0xf);

}


static Value Nor(list<Wire*> inputs)
{
   int output=U;
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


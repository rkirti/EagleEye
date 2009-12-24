#include "circuit.h"
#include <queue>
#include  <fstream>
using namespace std;


ofstream CIRCUIT_DFILE;
extern ofstream EVALUATE_DFILE;
static int cktDebug=1;
#define ERROR(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n");exit(0);}  else exit(0);}
#define DEBUG(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n")}





#define STRHELPER(x) #x
#define STRING(x) STRHELPER(x)
#define JOIN(x,y) STRING(x##y)




#define DERROR      {printf("Error.Cannot open debug file\n"); exit(0);}


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



bool Circuit::Simulate_Good()
{
    ifstream inputFile;
    ofstream outputFile;
    string inName;
    string outName;
    int inputval;

    list<Element*>::iterator innerIter;

    CIRCUIT_DFILE << "Starting simulation" << endl;
    cout << "Please enter the name of the file with input vectors " << endl;
    
    // Input vectors can be only 0,15,5 standing for 0,1,unknown
    cin >> inName;
    inputFile.open(inName.c_str(),ios::in);
    

    CIRCUIT_DFILE << "Reading input values from file " << inName <<  endl;
        
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        if ( inputFile >> inputval && (inputval == 0 || inputval == 15 || inputval == 5) )
        {
            iter->second->value = (Value) inputval;
            CIRCUIT_DFILE << "Setting  value of " <<  iter->second->id << " to  "  << inputval <<  endl;   
            
            // If the PI has fanout, then all branches should also be set to the
            // same value
            if (iter->second->outputs.size() > 1 )
            {
                innerIter = iter->second->outputs.begin();
                for ( ;innerIter != iter->second->outputs.end();innerIter++)
                {

                    CIRCUIT_DFILE << "Setting  value of branch " <<  (*innerIter)->id  << " of  stem "<<  iter->second->id << " to  "  << inputval <<  endl;   
                   ((Wire*) (*innerIter))->value = (Value) inputval; 
                }


            }


        }
        else 
        {
            CIRCUIT_DFILE << "Unacceptable  value of " <<  iter->second->id << " is  "  << inputval <<  endl;   
            CIRCUIT_DFILE << "Exiting" << endl;
            cout << "Unacceptable  value of " <<  iter->second->id << " is  "  << inputval <<  endl;   
            exit(0);
        }
    }

    CIRCUIT_DFILE << "Accepted all inputs, now calling circuit evaluate" << endl;
    CIRCUIT_DFILE << endl << endl;
    
    circuit.Evaluate();
    CIRCUIT_DFILE << "Evaluation done. Now printing outputs" << endl;
    cout << "Please enter the name of the file to print output vectors " << endl;
    
    // Input vectors can be only 0,15,5 standing for 0,1,unknown
    cin >> outName;
    outputFile.open(outName.c_str(),ios::out);
    
    
    iter =  (circuit.PriOutputs).begin();
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {
       outputFile << iter->second->value << endl;   
    }


    CIRCUIT_DFILE << endl << endl;
    
    outputFile.close();
    inputFile.close();

    return true;
}


bool Circuit::Evaluate()
{
    int level=1;
    multimap<int,Element*>:: iterator levelIter;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    CIRCUIT_DFILE << "Circuit evaluation called for" << endl;

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
                CIRCUIT_DFILE << "Setting value of wire " << outputWire->id << " to " << (int)curValue << endl; 
                outputWire->value = curValue;
                if  ((outputWire->outputs).size() > 1) 
                {
                    list<Element*>::iterator iter = (outputWire->outputs).begin();
                    CIRCUIT_DFILE << "Evaluating stemouts of wire " << outputWire->id << endl;
                    /* Reasons for the dynamic cast: 
                     * If the output wire has > 1 output, they are all bound to
                     * be wires.
                     * */
                    for (;iter != (outputWire->outputs).end(); iter++)
                    {
                        cout << "output: " << (*iter)->id << endl;
                        Wire* check = (dynamic_cast<Wire*>(*iter));
                        assert(check);
                        CIRCUIT_DFILE << "Setting value of branch " <<  check->id <<   " of wire " << outputWire->id << " to "  << (int) curValue << endl;
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

// NOTE:  We add only PIs, POs, and intermediate gates to the levels

bool Circuit::Levelize()
{

    int level=0;
    map<string,Wire*>::iterator iter =  (circuit.PriInputs).begin();
    multimap<int,Element*>:: iterator levelIter;

    CIRCUIT_DFILE << "Starting Levelize" << endl;

    // Adding the primary inputs to level zero
    CIRCUIT_DFILE <<  "Adding all primary inputs" << endl;
    
    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        CIRCUIT_DFILE << "Adding  " << iter->second->id << " at level 0 " << endl;
        (circuit.Levels).insert(pair<int,Element*>(0,(Element*)iter->second));
    }

    do
    {
        for (levelIter = ((circuit.Levels).equal_range(level)).first; 
                levelIter !=((circuit.Levels).equal_range(level)).second; levelIter++)
        {
            if ( (levelIter->second)->type == GATE)
            {
                Gate* curGate = dynamic_cast<Gate*>(levelIter->second);
                Wire* curWire = curGate->output;
                
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        CIRCUIT_DFILE << "Adding  " << curGate->id << " at level  " << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }
            }
            else if  ( (dynamic_cast<Wire*>(levelIter->second))->wtype == PI )
            {
                Wire* curWire =   dynamic_cast<Wire*>(levelIter->second);
                list<Element*>::iterator iter = (curWire->outputs).begin();
                for (;iter != (curWire->outputs).end(); iter++)
                {
                    Gate*  curGate = dynamic_cast<Gate*>(*iter);
                    if (!curGate) continue; 
                    curGate->tempInputs--;
                    if (curGate->tempInputs == 0) 
                    {

                        CIRCUIT_DFILE << "Adding  " << curGate->id << " at level  " << level+1 << endl;
                        (circuit.Levels).insert(pair<int,Element*>(level+1,(Element*)curGate));
                    }
                }

            } 

            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 
    CIRCUIT_DFILE << "Levelization completed" << endl << endl;
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
   CIRCUIT_DFILE << "Resolve Branches started" << endl;
   while (iter != (circuit.Netlist).end())
    {
        Wire* iwire = iter->second;
        
        // Condition for stemout
        // 1. If the wire is not a PO, output size is > 1
        // 2. If the wire is a PO output size is > 0
        if (  ( (int)(iwire->outputs).size() > 1 
            || ( (int) (iwire->outputs).size() > 0  && (iwire->wtype == PO))) 
            && (Wire_Not_Derived(iwire)))
        {

                CIRCUIT_DFILE << "Calling resolve wire for  " << iwire->id << endl;
                ResolveWire(iwire);
        }
        else 
        {
            CIRCUIT_DFILE << "No need to resolve wire " << iwire->id << endl;
        }
        iter++;
    }
    CIRCUIT_DFILE << "Resolve branches completed successfully" << endl << endl;
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
    if (wire->id.find("_") != string::npos ) return false;
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


void Circuit::Init_Debug()
{
    CIRCUIT_DFILE.open("debug/ckt.debug",ios::out);   
    EVALUATE_DFILE.open("debug/eval.debug",ios::out);   
}


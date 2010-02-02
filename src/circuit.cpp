#include "circuit.h"
#include <queue>
#include  <fstream>
using namespace std;


ofstream CIRCUIT_DFILE;
ifstream faultsFile;
extern ofstream EVALUATE_DFILE;
extern ofstream ATPG_DFILE;
extern ofstream MAIN_DFILE;



static int cktDebug=1;
#define ERROR(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n");exit(0);}  else exit(0);}
#define DEBUG(...) {if (cktDebug) {printf(__VA_ARGS__); printf("\n")}





#define STRHELPER(x) #x
#define STRING(x) STRHELPER(x)
#define JOIN(x,y) STRING(x##y)




#define DERROR      {printf("Error.Cannot open debug file\n"); exit(0);}












bool Circuit::Evaluate()
{
    int level=1;
    multimap<int,Element*>:: iterator levelIter;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    CIRCUIT_DFILE << "Circuit evaluation called for" << endl;

    // first propage the values on primary inputs to the immediate brances
    for ( iter = (circuit.PriInputs).begin(); iter != (circuit.PriInputs).end(); iter++)
    {
        Wire *outputWire = (iter->second);
        Value curValue = outputWire->value;
                if  ((outputWire->outputs).size() > 1) 
                {
                    list<Element*>::iterator iterator = (outputWire->outputs).begin();
                    CIRCUIT_DFILE << "Evaluating stemouts of wire " << outputWire->id << endl;
                    /* Reasons for the dynamic cast: 
                     * If the output wire has > 1 output, they are all bound to
                     * be wires.
                     * */
                    for (;iterator != (outputWire->outputs).end(); iterator++)
                    {
                        //cout << "output: " << (*iterator)->id << endl;
                        Wire* check = (dynamic_cast<Wire*>(*iterator));
                        assert(check);
                        CIRCUIT_DFILE << "Setting value of branch " <<  check->id <<   " of wire " << outputWire->id << " to "  << (int) curValue << endl;
                        // if the value of the wire is already set, don't set it again. (might be becuase it is faulty)
                        if (check->value == U)
                            check->value = curValue;

                    }

                }
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
                CIRCUIT_DFILE << "Setting value of wire " << outputWire->id << " to " << (int)curValue << endl; 
                // if the value of the wire is already set, don't set it again. (might be becuase it is faulty)
                if (outputWire->value == U)
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
                        //cout << "output: " << (*iter)->id << endl;
                        Wire* check = (dynamic_cast<Wire*>(*iter));
                        assert(check);
                        CIRCUIT_DFILE << "Setting value of branch " <<  check->id <<   " of wire " << outputWire->id << " to "  << (int) curValue << endl;
                        // if the value of the wire is already set, don't set it again. (might be becuase it is faulty)
                        if (check->value == U)
                            check->value = curValue;

                    }

                }

            }
            else cout << "something wrong" << endl;


        }
    }while (   (circuit.Levels).find(++level) != (circuit.Levels).end() ); 

    iter = (circuit.PriOutputs).begin();
    
//    for (;iter != (circuit.PriOutputs).end(); iter++)
//    {   
//         cout << "PO: " << (iter->second)->id << "value:  " << (iter->second)->value << endl;
//    }

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
    CIRCUIT_DFILE << "Print_All_Wires called"  << endl;


    // Print out the primary inputs
    CIRCUIT_DFILE << "Printing all the PIs "   << endl << endl;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    for (;iter != (circuit.PriInputs).end(); iter++)
    {   
        CIRCUIT_DFILE << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the primary outputs
    iter =  (circuit.PriOutputs).begin();
    CIRCUIT_DFILE << "Printing all POs " << endl << endl;
    for (;iter != (circuit.PriOutputs).end(); iter++)
    {   
        CIRCUIT_DFILE  << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }


    // Print out the netlist
    iter =  (circuit.Netlist).begin();
    CIRCUIT_DFILE << "Printing out the netlist " << endl << endl;
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        CIRCUIT_DFILE << setw(20) << (iter->second)->id << ":    " << (iter->second)->value << endl;
    }

    CIRCUIT_DFILE <<  endl << endl;
}


void Circuit::Init_Debug()
{
    //CIRCUIT_DFILE.open("/dev/null",ios::out);   
    //EVALUATE_DFILE.open("/dev/null",ios::out);   
    //ATPG_DFILE.open("/dev/null",ios::out);   
    //MAIN_DFILE.open("/dev/null",ios::out);   

    CIRCUIT_DFILE.open("debug/ckt.debug",ios::out);   
    EVALUATE_DFILE.open("debug/eval.debug",ios::out);   
    ATPG_DFILE.open("debug/atpg.debug",ios::out);   
    MAIN_DFILE.open("debug/main.debug",ios::out);   
}


/*
 * Clear all wire values to U
 */
void Circuit::Clear_Wire_Values()
{

    // Print out the netlist
    // Clear the values of all the wires. Set the value to U
    map<string,Wire*> ::iterator iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        (iter->second)->value = U;
    }
}



/*
 * Clear all internal wire values(all wires except PIs) to U
 */
void Circuit::Clear_Internal_Wire_Values()
{

    // Print out the netlist
    // Clear the values of all the wires. Set the value to U
    map<string,Wire*> ::iterator iter =  (circuit.Netlist).begin();
    for (;iter != (circuit.Netlist).end(); iter++)
    {   
        if ( (iter->second)->wtype != PI)
            (iter->second)->value = U;
    }
}

bool Circuit::ReadFaults()
{
    
    faultsFile.open("tests/faults.txt",ios::in);
    // Read faults from the faults.txt file
    if (!faultsFile.good())
    {
        cout << "Couldn't open the file tests/faults.txt" << endl;
        exit(-1);
    }
    
    string wireName;
    int faultType;
    map<string,Wire *>::iterator iter;

    while (faultsFile >> wireName >> faultType)
    {
        
        cout << "Fault specified: Wire: " << wireName << " FaultValue " << faultType << endl;
        if ( ((iter = Netlist.find(wireName)) !=  Netlist.end()) &&  (faultType == 0 || faultType == 1))
        {
            Fault readFault(iter->second, (faultType==0)?0:1);
            FaultSet.push_back(readFault);
        }
        else
        {
            cout << "Fault specified in tests/faults.txt incorrect - wire not found or faultvalue incorrect" << endl; 
            exit(-1);
        }
    }

    faultsFile.close();
    return true;
}

vector<Value> Circuit::CaptureOutput()
{
    vector<Value> output;
    map<string,Wire *>::iterator iter = PriOutputs.begin();
    for (;iter != PriOutputs.end(); iter++)
        output.push_back((iter->second)->value);
    return output;
}



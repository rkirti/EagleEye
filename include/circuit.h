/*
 * CIRCUIT Related functions for class Circuit
 */

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <iostream>                                               
#include <iomanip>                                               
#include <list>                                                   
#include <queue>                                                   
#include <map>
#include <string>                                                 
#include <cstdlib>                                                
#include <cassert>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <vector>



using namespace std;

enum Value{ZERO=0b0000,ONE=0b1111,U=0b0101,D=0b1100,DBAR=0b0011,ZEROBYU=0b0001,UBYZERO=0b0100,ONEBYU=0b1101,UBYONE=0b0111,UBAR=0b1010};

const Value ControlValues[7] = {ZERO,ONE,U,ZERO,ONE,U,U};

const Value InversionValues[7] = {ZERO,ZERO,U,ONE,ONE,U,U};

enum CktElement{WIRE=0,GATE,UNKNOWN};

enum GateType{AND=0,OR,NOT,NAND,NOR,XOR,BUF};

//const char* Evals[] = {AND,OR,};

enum WireType{PI=0,PO=1,CONNECTION=2};

class Wire;

/*Generic Evaluate function table*/
typedef Value (*GateEvaluate)(list<Wire*> inputs);



/* The evaluate functions array */
extern const GateEvaluate g_EvaluateTable[];






class Element
{
    public:
        string id;
        CktElement type; 

        /*Class element is an abstract class*/
        virtual Value Evaluate()=0;
        Element(const char* s, CktElement givenType=UNKNOWN) 
        { 
            id=s;
            type = givenType;
        }
};


class Wire: public Element
{
    public:
        Value value; 
        WireType wtype;
        virtual Value Evaluate()	// modify later, without defining this I am gettting compilation error.
	{
		return U;
	}
        /* Gate has only one output 
         * and wire has only one input*/
        Element* input;
        list<Element*> outputs;

	// This bit is set whenever the value is modified
	// This will be useful to eliminate further logging 
	// after it's first overwrite.
	bool modified;
        Wire(const char *name, WireType givenWtype,Value val=U)
           :Element(name,WIRE) 
        {
	        wtype = givenWtype;
            	value = val;
		modified=false; 	// The wire value is not modified yet
        }
};


class Gate: public Element
{
    public:
    GateType gtype;
    virtual Value Evaluate();
 
    list<Wire*> inputs;
    Wire* output;

    int level;	// The level of the gate, initially should be zero and is assigned after calling Levelize()

    int tempInputs; // needed by levelise
    Gate( char* name, GateType givenType) 
    	:Element(name,GATE)
    {  
        gtype = givenType;
        level = 0;
    } 
};


class Fault{
    public:
    Wire *FaultSite;
    bool faultType;     // s-a-0 or s-a-1
    int level;

    //Constructor
    Fault(Wire *faultwire,int type)
    {
        if ((type != 0) && (type != 1))
        {
            cout << "Unknown fault type" << endl;
            assert(false);
        }
        FaultSite = faultwire;
        faultType = type;
        level = 0;
    }

};



/* Instead of adding gates to D or J Frontiers,
 * we add their output wires
 * */
class WireValuePair{
    public:
    Wire*  iwire;
    Value value;
    WireValuePair(Wire *inputwire,Value val)
    {
        iwire = inputwire;
        value = val;
    }
};


class Implication{
public: 
    Wire* wire;
    Value value;
    bool direction; /* 0 backward,1 forward*/
    Implication(Wire* iwire,Value ival,bool dir)
    {
        wire = iwire;
        value =ival;
        direction = dir;
    }
};




class Circuit {

friend class ATPG;    
    
public:
    
    /* Stuff needed for basic circuit description
     * Parsing and evaluation*/
    map<string,Wire *> PriInputs; // Set of primary inputs
    map<string,Wire *> PriOutputs; //Set of primary outputs
    map<string,Wire *> Netlist;	// All wires in the circuit
    map<string,Gate *> Gates;   //All the gates in the circuit

    //queue<Implication*> ImpliQueue;


    /* This headache added to take care of a special case in renaming fanout
     * wires. 
     * If the same wire branches and two or more branches go to the same gate, 
     * we need this to name the wires
     */
    map<string,int> RepeatInputs; 




    multimap<int,Element*> Levels;

    list<Fault> FaultSet; // Set of faults we need to run ATPG for
    

    list<WireValuePair> DFrontier;
    list<WireValuePair> JFrontier;
    
    list<WireValuePair> TempDFrontier;
    list<WireValuePair> TempJFrontier;


    // The current fault under consideration
    Wire *faultWire;

    // Function, which adds a wire to the J frontier, along with it's value
 //   bool Add_To_JFrontier(Wire *wire,Value);
  //  bool Add_To_DFrontier(Wire *wire,Value);

 //   bool Remove_From_J(Wire *wire);
  //  bool Remove_From_D(Wire *wire);


    /*this should move to a different header file*/
    bool AddWire(const char* name,WireType type);
    /*Signals: input and output signals of the gate*/
    bool AddGate(GateType type, char *name,char* output,char **inputs,int numSignals);
    void Add_Gate_To_Wire_Output(Gate* gate,const char* wirename);   
    void  Add_Gate_To_Wire_Input(Gate* gate,const char* wirename);
    bool Levelize();
    bool ResolveBranches();
    void ResolveWire(Wire* somewire);

    void Update_Wire_Pair(Wire* oldwire,Wire* newwire);
    void  Update_Gate_Input(Gate* gate, Wire* oldwire,Wire* newwire);

    string Check_Name_Present(string givenname);
    string intToString(int inInt);

    bool Evaluate();
    vector<Value> CaptureOutput();
    bool Wire_Not_Derived(Wire* wire);
    void Print_All_Wires();
    void Clear_Wire_Values();
    void Clear_Internal_Wire_Values();
    void Init_Debug();
    bool Simulate_Good();
    bool ReadFaults();

};


/* The global circuit */
extern Circuit circuit;

bool isNotKnown(Value);
Value Do_Xor(Value val1, Value val2);
int Translate_Value_To_Int(Value value);


#include "evaluate.h"
#endif /* ifndef CIRCUIT_H */

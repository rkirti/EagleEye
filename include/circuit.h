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
#include  "enumdef.h"
using namespace std;

class Wire;
class Fault;

typedef list<Fault> FaultSet;

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
        virtual Value Evaluate()	
        // Not used. Any class inheriting Element has to define this.
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

 //   FaultSet set; // Set of faults we need to run ATPG for
    

    list<WireValuePair> DFrontier;
    list<WireValuePair> JFrontier;
    
    list<WireValuePair> TempDFrontier;
    list<WireValuePair> TempJFrontier;


    // The current fault under consideration
    Wire *faultWire;

 

    vector<Value> CaptureOutput();
    vector<Value> CaptureInput();
    //bool ReadFaults();

};



/* The global circuit */
extern Circuit circuit;

#include "evaluate.h"
#include "helper.h"
#include "setup.h"
#include "simulate.h"
#include "fault.h"
#include "test.h"
#endif /* ifndef CIRCUIT_H */

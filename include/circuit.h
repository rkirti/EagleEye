/*
 * CIRCUIT Related functions for class Circuit
 */

#ifndef CIRCUIT_H
#define CIRCUIT_H

#include <iostream>                                               
#include <list>                                                   
#include <map>
#include <string>                                                 
#include <cstdlib>                                                


using namespace std;

enum Value{ZERO=0b0000,ONE=0b1111,U=0b0101,D=0b1100,DBAR=0b0011,ZEROBYU=0b0001,UBYZERO=0b0100,ONEBYU=0b1101,UBYONE=0b0111};

enum CktElement{WIRE=0,GATE,UNKNOWN};

enum GateType{AND=0,OR,NOT,NAND,NOR,XOR};

//const char* Evals[] = {AND,OR,};

enum WireType{PI=0,PO,CONNECTION};

class Element
{
    public:
        string id;
        CktElement type; 

        /*Class element is an abstract class*/
        virtual Value Evaluate()=0;
        Element(char* s, CktElement givenType=UNKNOWN) 
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

        list<Element*> inputs;
        list<Element*> outputs;

        Wire(char *name, WireType givenWtype,Value val=U)
           :Element(name,WIRE) 
        {
	        wtype = givenWtype;
            value = val;
        }
};


class Gate: public Element
{
    public:
    GateType gtype;
    virtual Value Evaluate();
 
    list<Wire*> inputs;
    Wire* output;

    Gate(char* name, GateType givenType) 
    	:Element(name,GATE)
    {  
        gtype = givenType;
    } 
};


class Fault{
    Element *FaultSite;
    bool faultType;     // s-a-0 or s-a-1
};



class Circuit {
    
public:
    map<string,Wire*> PriInputs; // Set of primary inputs
    map<string,Wire*> PriOutputs; //Set of primary outputs
    map<string,Wire*> Netlist;	// All wires in the circuit
    map<string,Gate*> Gates;   //All the gates in the circuit
     
    Value*  testVector; // Array of values to be assigned to the pri inputs 
    //list<TestVector*> TestSet; // Final result  of test vectors generated
    list<Fault*> FaultSet; // Set of faults we need to run ATPG for
    
    /*this should move to a different header file*/
    bool AddWire(char* name,WireType type);
    /*Signals: input and output signals of the gate*/
    bool AddGate(GateType type, char *name,char* output,char **inputs,int numSignals);
    
    bool ResolveBranches();

};


/*Generic Evaluate function table*/
typedef Value (*GateEvaluate)(list<Wire*> inputs);

/* The evaluate functions array */
extern const GateEvaluate g_EvaluateTable[];

/* The global circuit */
extern Circuit circuit;

#endif /* ifndef CIRCUIT_H */

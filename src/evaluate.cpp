/** 
 * @file
 *  
 *  This file defines the evaluation functions for all the gate objects
 *  from the gate class. We use a function pointer table, indexing into
 *  it using the gate-type enum field in the gate class
 *  Finally, the file also contains a circuit evaluation function, which
 *  uses the levels field of each element in the circuit. The circuit
 *  is evaluated looping over all the levels -  an element can be evaluated
 *  only if its inputs, which are at a lower level than it have their
 *  values stable. The evaluate function checks if the faulty wire
 *  is set in the circuit, if so, its value is not changed while making 
 *  assignments.
 */




#include <iostream>
#include <fstream>
#include "circuit.h"
#include "evaluate.h"
using namespace std;


/// Defining the ***_DFILE debug files
ofstream EVALUATE_DFILE;
extern ofstream CIRCUIT_DFILE;



/// The function pointer table array. The indexes are set in the 
///  same order as the enum Gate Type defined in enumdefs.h
const GateEvaluate g_EvaluateTable[] = 
{
    And,
    Or,
    Not,
    Nand,
    Nor,
    Xor,
    Buf
};



    
/**
 * Generic Gate Evaluation Function
 * This invokes the appropriate function by indexing 
 * into the table using the gatetype field of the calling
 * gate object
 */

Value Gate::Evaluate()
{
    return g_EvaluateTable[gtype](inputs);
}





/**
 * AND gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the AND gate.
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to ONE initially following
 * the identity element for AND.
 */

Value And(list<Wire*> inputs)
{
   int output=ONE;	
   list<Wire*>::iterator iter;
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {
       output &= (*iter)->value;
   }
   return (Value)output;

}

/**
 * OR gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the OR gate.
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to ZERO initially following
 * the identity element for OR.
 */

Value Or(list<Wire*> inputs)
{
    int  output=ZERO;	
    list<Wire*>::iterator iter;

    for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {
           output |= (*iter)->value;
    }
    return (Value)output;

}

/**
 * BUF gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the gate.
 * Since this is a BUF gate, we assert that the
 * size of this list is one. The use of a list
 * cannot be avoided if compliance with the Gate class
 * has to be maintained.
 *
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to U initially.
 */



Value Buf(list<Wire*> inputs)
{
   Value output=U;
   list<Wire*>::iterator iter;
   assert(inputs.size() == 1);

   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {
           output = (*iter)->value;
    }
    return output;
}


/**
 * NOT gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the gate.
 * Since this is a NOT gate, we assert that the
 * size of this list is one. The use of a list
 * cannot be avoided if compliance with the Gate class
 * has to be maintained.
 *
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to U initially.
 */

Value Not(list<Wire*> inputs)
{
   Value output=U;
   list<Wire*>::iterator iter;
   assert(inputs.size() == 1);
   
   iter=inputs.begin();
   output = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
    
   return output;
}


/**
 * NAND gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the NAND gate.
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to ZERO initially following
 * the identity element for NAND.
 */

Value Nand(list<Wire*> inputs)
{
    int output=ZERO;
    list<Wire *>::iterator iter;
    Value curValue;

   /// Use DeMorgan's laws. They help maintain accuracy here
   /// since the negation in the end loses the Value information

    EVALUATE_DFILE << "Nand Evaluate called "  << endl;
    for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {    
        /// Care has to be taken in negating, since 
        /// the negation of U should give back U itself.       
        EVALUATE_DFILE << "Input wire is  " << (*iter)->id << " with value  " << (*iter)->value << endl; 
        curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;

        output |= curValue;
    }
    EVALUATE_DFILE << "Output is "  <<  (Value) output << endl;
    EVALUATE_DFILE << endl << endl;
    return (Value) output; 
}


/**
 * NOR gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the NOR gate.
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to ONE initially following
 * the identity element for NOR.
 */

Value Nor(list<Wire*> inputs)
{
   int output=ONE;
   list<Wire *>::iterator iter;
   Value curValue;
   
   
   
   /// Use DeMorgan's laws. They help maintain accuracy here
   /// since the negation in the end loses the Value information
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {  
       /// Care has to be taken in negating, since 
       /// the negation of U should give back U itself.       
       curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
       output &= curValue;
   }
   return (Value)(output);

}


/**
 * XOR gate evaluate function
 *
 * @param 
 * The list of pointers to Wire objects
 * which form the inputs of the XOR gate.
 *
 * @return 
 * The value at the gate output.
 * See the enum definition of Value in enumdefs.h
 *
 * The output is set to XOR initially following
 * the identity element for XOR.
 */
Value Xor(list<Wire *> inputs)
{
    int output=ZERO;
    list<Wire *>::iterator iter;
    Value curOutput;
    Value curNegOutput;
    Value curValue;
    Value curNegValue;


    for (iter=inputs.begin(); iter != inputs.end(); iter++)
    {


            curValue = (*iter)->value;   
            curNegValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
            curOutput = (Value)output;
            curNegOutput =(output != U)?(Value)(~output&0xf):U;

        output = ((output & curNegValue) | (curNegOutput & curValue));
    }
    return (Value)output;
}


/**
 * @param
 *
 * Reference to a circuit object.
 *
 * @return
 * Always returns true. The actual output is stored
 * in the values of the PriOutputs of the circuit.
 *
 * 
 */

bool Evaluate( Circuit& circuit)
{
    int level=1;
    multimap<int,Element*>:: iterator levelIter;
    map<string,Wire*> ::iterator iter =  (circuit.PriInputs).begin();

    CIRCUIT_DFILE << "Circuit evaluation called for" << endl;

    /// First propagate the values on primary inputs to the immediate branches. Not doing so leads to big bugs. Whenever  a value is assigned
    ///to a wire, check for its branches and assign values to those too.
    for ( iter = (circuit.PriInputs).begin(); iter != (circuit.PriInputs).end(); iter++)
    {
        Wire *outputWire = (iter->second);
        Value curValue = outputWire->value;
                if  ((outputWire->outputs).size() > 1) 
                {
                    list<Element*>::iterator iterator = (outputWire->outputs).begin();
                    CIRCUIT_DFILE << "Evaluating stemouts of wire " << outputWire->id << endl;
                    /** Reasons for using dynamic cast: 
                     * If the output wire has > 1 output, they are all bound to 
                     * be wires.
                     * */
                    for (;iterator != (outputWire->outputs).end(); iterator++)
                    {
                        Wire* check = (dynamic_cast<Wire*>(*iterator));
                        assert(check);
                        CIRCUIT_DFILE << "Setting value of branch " <<  check->id <<   " of wire " << outputWire->id << " to "  << (int) curValue << endl;
                        /// if the value of the wire is already set, don't set it again. (might be becuase it is faulty)
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
    
    return true;
}


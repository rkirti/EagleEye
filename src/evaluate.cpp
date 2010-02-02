/*
 * Gate Evaluation Functions
 */


#include <iostream>
#include <fstream>
#include "circuit.h"
#include "evaluate.h"
using namespace std;

ofstream EVALUATE_DFILE;
extern ofstream CIRCUIT_DFILE;


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





/*
 * The logic functions
 * AND, OR, NOT, NAND, NOR
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




Value Not(list<Wire*> inputs)
{
   Value output=U;
   list<Wire*>::iterator iter;
   assert(inputs.size() == 1);
   
   iter=inputs.begin();
   output = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
    
   return output;
}








Value Nand(list<Wire*> inputs)
{
    int output=ZERO;
    list<Wire *>::iterator iter;
    Value curValue;

    // Use DeMorgan's laws. They help maintain accuracy here
    // since the negation in the end loses the Value information

    EVALUATE_DFILE << "Nand Evaluate called "  << endl;
    for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {    
        EVALUATE_DFILE << "Input wire is  " << (*iter)->id << " with value  " << (*iter)->value << endl; 
        curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;

        output |= curValue;
    }
    EVALUATE_DFILE << "Output is "  <<  (Value) output << endl;
    EVALUATE_DFILE << endl << endl;
    return (Value) output; 
}


Value Nor(list<Wire*> inputs)
{
   int output=ONE;
   list<Wire *>::iterator iter;
   Value curValue;
   
   
   
   // Use DeMorgan's laws. They help maintain accuracy here
   // since the negation in the end loses the Value information
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {    
           curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
       output &= curValue;
   }
   
   
   return (Value)(output);

}


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


bool Evaluate( Circuit& circuit)
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


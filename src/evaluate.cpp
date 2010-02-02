/*
 * Gate Evaluation Functions
 */


#include <iostream>
#include <fstream>
#include "circuit.h"
#include "evaluate.h"
using namespace std;

ofstream EVALUATE_DFILE;

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



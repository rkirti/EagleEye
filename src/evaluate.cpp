/*
 * Gate Evaluation Functions
 */


#include <iostream>
#include "circuit.h"
#include "evaluate.h"
using namespace std;


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
   // Initial value must be U
   // Note: output is set to type int and 
   // not Value to keep the compiler happy
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
   // Initial value must be U
   // Note: output is set to type int and 
   // not Value to keep the compiler happy
   int  output=ZERO;	
   list<Wire *>::iterator iter;
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output |= (*iter)->value;
    return (Value)output;

}


Value Buf(list<Wire*> inputs)
{
   Value output=U;
   assert(inputs.size() == 1);

   list<Wire *>::iterator iter;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
       output = (*iter)->value;
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
   
   // Use DeMorgan's laws. They help maintain accuracy here
   // since the negation in the end loses the Value information
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {    
       // Take care of the fact that UBAR is U
       Value curValue = ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
       output |= curValue;
   }
       
   return (Value) output; 
}


Value Nor(list<Wire*> inputs)
{
   int output=ONE;
   list<Wire *>::iterator iter;
   
   
   // Use DeMorgan's laws. They help maintain accuracy here
   // since the negation in the end loses the Value information
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {    
       // Take care of the fact that UBAR is U
       Value curValue = ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
       cout << "Input value: " << (*iter)->value << endl;
       output &= curValue;
   }
   
   
   return (Value)(output);

}


Value Xor(list<Wire *> inputs)
{
	int output=ZERO;
	list<Wire *>::iterator iter;



	for (iter=inputs.begin(); iter != inputs.end(); iter++)
    {
        Value curValue = (*iter)->value;
        Value curNegValue =  ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
        Value curOutput = (Value)output;
        Value curNegOutput =(output != U)?(Value)(~output&0xf):U;

		output = ((output & curNegValue) | (curNegOutput & curNegValue));
    }
	return (Value)output;
}



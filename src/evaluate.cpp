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
   Implication* intention=NULL;
   list<Wire*>::iterator iter;
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {
       if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
            output &= intention->value;
       else
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
    Implication* intention=NULL;
    list<Wire*>::iterator iter;

    for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {
        if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
            output |= intention->value;
        else
            output |= (*iter)->value;
    }
    return (Value)output;

}



Value Buf(list<Wire*> inputs)
{
   Value output=U;
   Implication* intention=NULL;
   list<Wire*>::iterator iter;
   assert(inputs.size() == 1);

   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
    {
        if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
            output = intention->value;
        else
            output = (*iter)->value;
    }
    return output;
}




Value Not(list<Wire*> inputs)
{
   Implication* intention=NULL;
   Value output=U;
   list<Wire*>::iterator iter;
   
   assert(inputs.size() == 1);
   iter=inputs.begin();
    
   if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
	{
	    cout << __LINE__ << " found not gate in impli queue, value : " << intention->value  << endl;
            output =(Value) (intention->value != U)?(Value)(~(intention->value)&0xf):U;
	}
   else
   { 
            cout << __LINE__ <<  " not found Not gate in impli queue " << endl;
            output = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
   }

   cout << __LINE__ << " output from not gate = " << output << endl;
   //cout << __LINE__ << " current output from not gate = " << ( (*iter)->outputs.begin())->value << endl;
    
   return output;
}








Value Nand(list<Wire*> inputs)
{
   int output=ZERO;
   Implication* intention=NULL;
   list<Wire *>::iterator iter;
   Value curValue;

   // Use DeMorgan's laws. They help maintain accuracy here
   // since the negation in the end loses the Value information
   
  cout << "****************DEBUG***********"  << endl;
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {    
       // Take care of the fact that UBAR is U
     if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
     { 
        cout << "Intention selected as curValue"<< endl; 
         curValue =(Value) (intention->value != U)?(Value)(~(intention->value)&0xf):U;
    } 
        else
        {

            cout << "Wire value selected as curValue"<< endl; 
            curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
        } 
    cout << "neg curValue for " << (*iter)->id  << "is " << curValue << endl; 
     output |= curValue;
   }
   cout << "Output is "  <<  (Value) output << endl;
   return (Value) output; 
}


Value Nor(list<Wire*> inputs)
{
   int output=ONE;
   Implication* intention=NULL;
   list<Wire *>::iterator iter;
   Value curValue;
   
   
   
   // Use DeMorgan's laws. They help maintain accuracy here
   // since the negation in the end loses the Value information
   
   for (iter=inputs.begin(); iter != inputs.end(); iter++ )
   {    
       // Take care of the fact that UBAR is U
      if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
            curValue =(Value) (intention->value != U)?(Value)(~(intention->value)&0xf):U;
     else
            curValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
       output &= curValue;
   }
   
   
   return (Value)(output);

}


Value Xor(list<Wire *> inputs)
{
	int output=ZERO;
   Implication* intention=NULL;
	list<Wire *>::iterator iter;
    Value curOutput;
    Value curNegOutput;
    Value curValue;
    Value curNegValue;


    for (iter=inputs.begin(); iter != inputs.end(); iter++)
    {


        if ( (intention = Find_In_Intentions_List((*iter))) != NULL)
        {
            curValue = (intention)->value;
            curNegValue =(Value) (intention->value != U)?(Value)(~(intention->value)&0xf):U;
        }
        else
        {
            curValue = (*iter)->value;   
            curNegValue = (Value) ((*iter)->value != U)?(Value)(~((*iter)->value)&0xf):U;
        }  

        curOutput = (Value)output;
        curNegOutput =(output != U)?(Value)(~output&0xf):U;

        output = ((output & curNegValue) | (curNegOutput & curNegValue));
    }
    return (Value)output;
}



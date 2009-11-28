
/*
 * ATPG Related functions for class Circuit
 */
#ifndef ATPG_H
#define ATPG_H

#include "lexer.h"
#include "circuit.h"

class ATPG{

friend class Circuit;

public:
    bool Do_ATPG();
    bool D_Algo();


    bool Imply_And_Check();
    bool Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value curValue);
    bool Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value curValue);

    bool Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value curValue);
    bool Handle_Output_Coming_From_Noncontrol_Value(Implication* curImplication, Wire* curWire,Value curValue);

    bool Add_To_JFrontier(Wire *wire,Value value);
    bool Add_To_DFrontier(Wire *wire,Value value);
    
    bool RemoveFromD(Wire *wire);
    bool RemoveFromJ(Wire *wire);

    bool Compatible(Value oldval,Value newval);
    bool  Check_Wire_Value(Wire* wire,Value val);
    
    void Make_Assignments();
    void Update_PI_For_9V();

    void Failure();



};



#endif /* ifndef ATPG_H */


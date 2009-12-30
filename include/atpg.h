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
    bool Do_ATPG(Wire *faultwire, Value faultval);
    bool D_Algo();


    bool Imply_And_Check();
    bool Resolve_Forward_Implication(Implication* curImplication,Wire* curWire, Value curValue);
    bool Resolve_Backward_Implication(Implication* curImplication,Wire* curWire, Value curValue);

    bool Handle_Output_Coming_From_Control_Value(Implication* curImplication, Wire* curWire,Value curValue);
    bool Handle_Output_Coming_From_Noncontrol_Value(Implication* curImplication, Wire* curWire,Value curValue);
    bool Handle_Forward_Implication_On_Stem (Implication* curImplication, Wire* curWire,Value curValue);

    bool Add_To_JFrontier(Wire *wire,Value value);
    bool Add_To_DFrontier(Wire *wire,Value value);
    
    bool Remove_From_D(Wire *wire);
    bool Remove_From_J(Wire *wire);

    bool Compatible(Value oldval,Value newval);
    bool  Check_Wire_Value_For_Assignment(Wire* wire,Value val);
    
//    void Make_Assignments();
    void Clean_Logs();
    void Update_PI_For_9V();

    void Failure();


    Implication* Find_In_Logs_List(Wire* wire);

};

bool Change_Value_And_Update_Log (Implication *);

static list<Implication*> Logs;
static queue<Implication*> ImpliQueue;

#endif /* ifndef ATPG_H */


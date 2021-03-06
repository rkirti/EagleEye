/*
 * ATPG Related functions for class Circuit
 */

#ifndef ATPG_H
#define ATPG_H

#include "lexer.h"
#include <vector>
#include <time.h>
#include "circuit.h"


typedef vector<Value> TestVector;
typedef  list<TestVector> TestSet;

//! Test Generator class that uses the D-Algorithm 
class ATPG : public Test
{

friend class Circuit;

public:
    TestSet PerformTest(Circuit& circuit,FaultSet set);
 
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
    
    void Merge_Frontiers();
    
    bool Remove_From_D(Wire *wire);
    bool Remove_From_J(Wire *wire);
    
    bool Compatible(Value oldval,Value newval);
    bool Check_Wire_Value_For_Assignment(Wire* wire,Value val);
    
    void Clean_Logs();

    /// Deprecated function - use back again if you switch to 9V
    void Update_PI_For_9V();

    void Failure();


};

class RandomVectorTest
{
    public:
    RandomVectorTest();
    ~RandomVectorTest();
    void PerformTest(int coverage,int timeLimit);
    void GenerateAndSetRVector();
};


// Both ATPG and Random Test need this - 
// so keeping it out of both classes
void Generate_Full_FaultSet();
bool Change_Value_And_Update_Log (Implication *);

static list<Implication*> Logs;
static queue<Implication*> ImpliQueue;

#endif /* ifndef ATPG_H */


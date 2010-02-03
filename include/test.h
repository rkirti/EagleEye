
/*
 * Generic Test Routines to be used by all ATPG classes
 */
#ifndef TEST_H
#define TEST_H

#include "circuit.h"
#include "fault.h"


typedef vector<Value> TestVector;

typedef  list<TestVector> TestSet;


class Test{

public:
    FaultSet faults;
    TestSet  tests;
// Data:
// FaultSet - list of fault objects
// TestVectors - list of test vector objects


// Functions:
// 0. If required auto generate the fault set - do this in various ways
// 1. Read inputs - input is the fault set to read - store this in your fault
// set
// 2. Reduce the faultset wherever possible
// 3. For the faultset obtained, run the algo using PerformTest, which is a
// virtual function defined by all classes inheriting this class
// 4. As you keep performing the test, store a multimap of test vectors
// to fault pointers
// 5. Optimization step - minimize the test vector set needed
// 6. Print out the results to a file 
    bool PerformTest;

    void Print_Test_Set();

};



#endif /* ifndef TEST_H */


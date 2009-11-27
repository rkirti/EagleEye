
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
    void Update_PI_For_9V();
};



#endif /* ifndef ATPG_H */


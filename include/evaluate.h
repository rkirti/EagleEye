/*
 * EVALUATION FUNCTIONS AND WRAPPERS
 */

#ifndef EVALUATE_H
#define EVALUATE_H

#include "circuit.h"
#include "atpg.h"

Implication* Find_In_Intentions_List(Wire* wire);


Value And(list<Wire *> inputs);
Value Or(list<Wire *> inputs);
Value Not(list<Wire *> inputs);
Value Nand(list<Wire *> inputs);
Value Nor(list<Wire *> inputs);
Value Xor(list<Wire *> inputs);
Value Buf(list<Wire *> inputs);



bool Evaluate( Circuit& circuit);

#endif /* ifndef EVALUATE_H */

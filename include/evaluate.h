/*
 * EVALUATION FUNCTIONS AND WRAPPERS
 */

#ifndef EVALUATE_H
#define EVALUATE_H

#include "circuit.h"




Value And(list<Wire *> inputs);
Value Or(list<Wire *> inputs);
Value Not(list<Wire *> inputs);
Value Nand(list<Wire *> inputs);
Value Nor(list<Wire *> inputs);
Value Xor(list<Wire *> inputs);
Value Buf(list<Wire *> inputs);



#endif /* ifndef EVALUATE_H */

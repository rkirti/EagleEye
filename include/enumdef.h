/*
 * Enum Defintions
 */
#ifndef ENUMDEF_H
#define ENUMDEF_H


enum Value{ZERO=0b0000,ONE=0b1111,U=0b0101,D=0b1100,DBAR=0b0011,ZEROBYU=0b0001,UBYZERO=0b0100,ONEBYU=0b1101,UBYONE=0b0111,UBAR=0b1010};

const Value ControlValues[7] = {ZERO,ONE,U,ZERO,ONE,U,U};

const Value InversionValues[7] = {ZERO,ZERO,U,ONE,ONE,U,U};

enum CktElement{WIRE=0,GATE,UNKNOWN};

enum GateType{AND=0,OR,NOT,NAND,NOR,XOR,BUF};

enum WireType{PI=0,PO=1,CONNECTION=2};


#endif /* ifndef ENUMDEF_H*/

/*
 * Structure and Functions definitions for the ones 
 * used in parsing and nowhere else
 */
#ifndef DEFS_H
#define DEFS_H


typedef struct namenode{
    char* name;
    struct namenode* next;
}Namenode;

typedef struct gatenode{
    Namenode* output;
    Namenode* inputlist;
}Gatenode;





#endif /* ifndef DEFS_H */


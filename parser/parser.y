%{
/* Simple parser to populate the ckt using
 * the verilog description
 * */

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "circuit.h"

#define YYDEBUG 1

extern char* yytext;
extern FILE* yyin;
extern int yydebug;


FILE* infp;

typedef struct namenode{
    char* name;
    struct namenode* next;
}Namenode;

typedef struct gatenode{
    Namenode* output;
    Namenode* inputlist;
}Gatenode;

%union{
    Namenode* namelist;
    Gatenode* gate;
    char letter;
    char* gatename;
    char letter;
    int id;
}


%}


%token T_ENDMODULE
%token T_MODULE
%token T_SEMICOLON  T_COMMA
%token T_INPUT T_OUTPUT
%token T_LPAREN T_RPAREN
%token T_WIRE
%token T_AND 
%token <gatename> T_AND_NAME 
%token T_NAND 
%token <gatename> T_NAND_NAME 
%token T_OR  
%token <gatename> T_OR_NAME 
%token T_NOT 
%token <gatename> T_NOT_NAME 
%token T_NOR 
%token <gatename> T_NOR_NAME 
%token T_BUF  
%token <gatename> T_BUF_NAME  
%token T_XOR  
%token <gatename> T_XOR_NAME


%type <gate> and or not nor nand xor buf 
%type <namelist> signallist


%%

ckt: module input output wire gatelist T_ENDMODULE
   ;


module:  T_MODULE name T_LPAREN signallist T_RPAREN T_SEMICOLON
   ;

input:  T_INPUT signallist T_SEMICOLON
   ;

output:  T_OUTPUT signallist T_SEMICOLON
   ;

wire:  T_WIRE signallist T_SEMICOLON
  ;  



gatelist: gatelist gate T_SEMICOLON
 | gate T_SEMICOLON
 ;

gate: and  
  | or
  | not 
  | nor 
  | nand
  | xor
  | buf
  ;

signallist: signallist T_COMMA name
  |name
  ;


output: name
  ;

and: T_AND T_AND_NAME T_LPAREN output T_COMMA signallist T_RPAREN
  ; 

nand: T_NAND T_NAND_NAME T_LPAREN output T_COMMA signallist T_RPAREN
  ;

or: T_OR T_OR_NAME T_LPAREN output T_COMMA signallist T_RPAREN
  ;

nor: T_NOR T_NOR_NAME T_LPAREN output T_COMMA signallist T_RPAREN
 ;

not: T_NOT T_NOT_NAME T_LPAREN output T_COMMA signallist T_RPAREN
 ;

xor: T_XOR T_XOR_NAME T_LPAREN output T_COMMA signallist T_RPAREN
;

buf:  T_BUF  T_BUF_NAME  T_LPAREN output T_COMMA signallist T_RPAREN
;
  

 /*Common name format for gates,wires and circuits */
name: NAME  
 ;

%%



int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("Usage: ./atpg <verilog_source> \n");
        exit(0);
    }
    
    infp = fopen(argv[1],"r");
       
    if (infp == NULL)
    {
        fprintf(stderr,"Could not open specified file: %s\n",argv[1]);
        exit(1);
    }
    
   yyin = infp;
   yydebug=1;
    
   while(!feof(yyin)){
        yyparse();
  }
    return 0;
}
  

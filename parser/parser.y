%{
/* Simple parser to populate the ckt using
 * the verilog description
 * */

#include <stdio.h>
#include <stdlib.h>

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

Namenode* Add_Name_To_List(Namenode* list, char* name)
{
}



%}


%union{
    Namenode* namelist;
    Gatenode* gate;
    char letter;
    char* gatename;
    char* genname;
}




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
%token <genname> T_NAME

%type <gate> and or not nor nand xor buf 
%type <namelist> signallist output
%type <genname> name

%%

ckt: module inputs outputs wire gatelist T_ENDMODULE
   ;


module:  T_MODULE name T_LPAREN signallist T_RPAREN T_SEMICOLON
   ;

inputs:  T_INPUT signallist T_SEMICOLON
   ;

outputs:  T_OUTPUT signallist T_SEMICOLON
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

signallist: signallist T_COMMA name   { $$ = Add_Name_To_List($1,$3); }
  |name  { $$ = Add_Name_To_List($$,$1); }
  ;


output: name  { $$ = Add_Name_To_List($$,$1); }
  ;

and: T_AND T_AND_NAME T_LPAREN output T_COMMA signallist T_RPAREN {$$->output = $4;$$->inputlist = $6;}
  ; 

nand: T_NAND T_NAND_NAME T_LPAREN output T_COMMA signallist T_RPAREN  {$$->output = $4;$$->inputlist = $6;}
  ;

or: T_OR T_OR_NAME T_LPAREN output T_COMMA signallist T_RPAREN   {$$->output = $4;$$->inputlist = $6;}
  ;

nor: T_NOR T_NOR_NAME T_LPAREN output T_COMMA signallist T_RPAREN    {$$->output = $4;$$->inputlist = $6;}
 ;


not: T_NOT T_NOT_NAME T_LPAREN output T_COMMA signallist T_RPAREN    {$$->output = $4;$$->inputlist = $6;}
 ;


xor: T_XOR T_XOR_NAME T_LPAREN output T_COMMA signallist T_RPAREN    {$$->output = $4;$$->inputlist = $6;}
;

buf:  T_BUF  T_BUF_NAME  T_LPAREN output T_COMMA signallist T_RPAREN    {$$->output = $4;$$->inputlist = $6;}
;
  

 /*Common name format for gates,wires and circuits */
name: T_NAME  
 ;

%%


yyerror(const char* arg)
{
   /*   printf("Error [Line %d]: %s",numLines,arg);*/
}



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
   yydebug=0;
    
   while(!feof(yyin)){
        yyparse();
  }
    return 0;
}
  



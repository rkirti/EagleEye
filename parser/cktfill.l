%{
    /* Basic lexer for EagleEye */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "defs.h"
#include "parser.tab.h"

#define YYDEBUG 1

#define Pass_Gate_Name()  { yylval.gatename=(char*)malloc(sizeof(char)*(strlen(yytext)+1));strcpy(yylval.gatename,yytext); }    
#define Pass_Gen_Name()  { yylval.genname=(char*)malloc(sizeof(char)*(strlen(yytext)+1));strcpy(yylval.genname,yytext); }    
extern YYSTYPE yylval;

%}


%%



endmodule {return T_ENDMODULE;}
module {return T_MODULE;}
wire  {return T_WIRE;} 
input {return T_INPUT;}
output {return T_OUTPUT;}
and {return T_AND;}
nand {return T_NAND;}
or  {return T_OR;} 
not {return T_NOT;}
nor {return T_NOR;}
buf {return T_BUF;}
xor {return T_XOR;}

";" {return T_SEMICOLON;}
"," {return T_COMMA;}
"(" {return T_LPAREN;}
")" {return T_RPAREN;}

AND[0-9]+_[0-9]+  { Pass_Gate_Name();return T_AND_NAME;}
NAND[0-9]+_[0-9]+  {  Pass_Gate_Name(); return T_NAND_NAME;}
OR[0-9]+_[0-9]+  {  Pass_Gate_Name(); return T_OR_NAME;}
NOR[0-9]+_[0-9]+  {  Pass_Gate_Name(); return T_NOR_NAME;}
NOT[0-9]*_[0-9]+  { Pass_Gate_Name(); return T_NOT_NAME;}
BUFF[0-9]+_[0-9]+  { Pass_Gate_Name(); return T_BUF_NAME;}
XOR[0-9]+_[0-9]+  { Pass_Gate_Name();return T_XOR_NAME;}
[A-Za-z]+[0-9]*   { Pass_Gen_Name(); return T_NAME;}


%%

int yywrap(){
    return 1;
}




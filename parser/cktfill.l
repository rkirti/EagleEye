%{
    /* Basic lexer for eagle eye */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "y.tab.h"

#define YYDEBUG 1

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

AND[0-9]+_[0-9]+  {yylval=yytext; return T_AND_NAME;}
NAND[0-9]+_[0-9]+  {yylval=yytext; return T_NAND_NAME;}
OR[0-9]+_[0-9]+  {yylval=yytext; return T_OR_NAME;}
NOR[0-9]+_[0-9]+  {yylval=yytext; return T_NOR_NAME;}
NOT[0-9]+_[0-9]+  {yylval=yytext; return T_NOT_NAME;}
BUF[0-9]+_[0-9]+  {yylval=yytext; return T_BUF_NAME;}
XOR[0-9]+_[0-9]+  {yylval=yytext; return T_XOR_NAME;}





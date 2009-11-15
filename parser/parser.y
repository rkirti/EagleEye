ckt: module input output wire gatelist T_ENDMODULE
   ;


module:  T_MODULE name T_LPAREN signallist T_RPAREN SEMICOLON

input:  T_INPUT signallist SEMICOLON


output:  T_OUTPUT signallist SEMICOLON


wire:  T_WIRE signallist SEMICOLON

gatelist: gatelist gate SEMICOLON
 | gate SEMICOLON
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
name: LETTER INT_ID
 ;


  

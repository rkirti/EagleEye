# The cpp source files
FILES = main.cpp evaluate.cpp circuit.cpp lexer.cpp atpg.cpp dot.cpp

# Absolute locations of the source and object files (wrt the Makefile location)
SRC_FILES := $(FILES:%.cpp=src/%.cpp)
OBJ_FILES := $(FILES:%.cpp=obj/%.o)

# Headers' location
HEADER_SRC=include/

# Binaries directory
BIN_DIR=bin

# Instrcution of how to compile the cpp source files
obj/%.o: src/%.cpp
	g++ -g -c $< -I $(HEADER_SRC) -o $@ 

# The main executable
atpg: $(OBJ_FILES) parser/parser.tab.o parser/lex.yy.o
	g++ -g $(OBJ_FILES) parser/parser.tab.o parser/lex.yy.o -o $(BIN_DIR)/$@ 

parser/parser.tab.o: parser/parser.tab.c parser/parser.tab.h
	gcc -g -c -I include/ -I parser/ parser/parser.tab.c -o parser/parser.tab.o
parser/parser.tab.c: parser/parser.y
	yacc -o parser/parser.tab.c -d parser/parser.y 

parser/lex.yy.o: parser/lex.yy.c parser/defs.h
	gcc -g -c -I include/ -I parser/ parser/lex.yy.c -o parser/lex.yy.o
parser/lex.yy.c: parser/cktfill.l
	lex -o parser/lex.yy.c parser/cktfill.l

# Remove the generated object files
clean: 
	rm -f obj/* $(BIN_DIR)/* parser/*.o parser/lex.yy.c parser/parser.tab.*

# Build the dependency of the cpp source files on the header files
depend: 
	g++ -g -I $(HEADER_SRC) -M $(SRC_FILES) \
		| perl -ne 's/^(\S)/obj\/$$1/;print' \
		> depend.mak

# Default rule to make depend.mak
depend.mak:
	touch $@

# Include the generated dependency file
include depend.mak

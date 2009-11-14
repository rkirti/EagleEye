# The cpp source files
FILES = main.cpp circuit.cpp lexer.cpp atpg.cpp

# Absolute locations of the source and object files (wrt the Makefile location)
SRC_FILES := $(FILES:%.cpp=src/%.cpp)
OBJ_FILES := $(FILES:%.cpp=obj/%.o)

# Headers' location
HEADER_SRC=include/

# Binaries directory
BIN_DIR=bin

# Instrcution of how to compile the cpp source files
obj/%.o: src/%.cpp
	g++ -c $< -I $(HEADER_SRC) -o $@

# The main executable
atpg: $(OBJ_FILES)
	g++ $(OBJ_FILES) -o $(BIN_DIR)/$@

# Remove the generated object files
clean: 
	rm -f obj/* $(BIN_DIR)/*

# Build the dependency of the cpp source files on the header files
depend: 
	g++ -I $(HEADER_SRC) -M $(SRC_FILES) \
		| perl -ne 's/^(\S)/obj\/$$1/;print' \
		> depend.mak

# Default rule to make depend.mak
depend.mak:
	touch $@

# Include the generated dependency file
include depend.mak

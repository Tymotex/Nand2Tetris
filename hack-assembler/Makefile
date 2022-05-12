# Compiles the C++ Hack Assembler.
CC = g++

HackAssembler: HackAssembler.o SymbolTable.o Parser.o Code.o Parser.h
	$(CC) -g -o HackAssembler HackAssembler.cc SymbolTable.o Parser.o Code.o

HackAssembler.o: HackAssembler.cc
	$(CC) -g -c HackAssembler.cc

SymbolTable.o: SymbolTable.cc SymbolTable.h
	$(CC) -g -c SymbolTable.cc

Parser.o: Parser.cc Parser.h
	$(CC) -g -c Parser.cc Parser.h

Code.o: Code.cc Code.h
	$(CC) -g -c Code.cc

# Testing
test:
	cmake -S . -B build > /dev/null && \
	cmake --build build > /dev/null && \
	cd build > /dev/null && \
	ctest

# Compiles the C++ Hack Assembler.
CC = g++

HackAssembler: HackAssembler.o SymbolTable.o Parser.o Code.o
	$(CC) -o HackAssembler HackAssembler.cc SymbolTable.o Parser.o Code.o

HackAssembler.o: HackAssembler.cc
	$(CC) -c HackAssembler.cc

SymbolTable.o: SymbolTable.cc SymbolTable.h
	$(CC) -c SymbolTable.cc

Parser.o: Parser.cc Parser.h
	$(CC) -c Parser.cc

Code.o: Code.cc Code.h
	$(CC) -c Code.cc


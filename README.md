# Hack & Jack 

A journey from NAND gates to Tetris, following implementation suggestions given
in ['The Elements of Computing Systems'](https://mitpress.mit.edu/books/elements-computing-systems-second-edition) by Noam Nisan and Shimon Schocken,
the book version of the [Nand2Tetris](https://www.nand2tetris.org/) course.

## Hack
Hack is a simple 16-bit general-purpose computer based on the Von Neumann architecture. It's
built using chips defined with a custom HDL language provided as part of
Nand2Tetris.

I started the journey from as low of a level as possible. Roughly, I learned
and did things in this order:
1. Silicon crystals and doping.
2. P-N junctions and diodes.
3. NPN/PNP transistors.
4. Logic gate implementation using transistors and resistors.
5. Logic design with primitive logic gates and the Hack HDL language.
6. Multiplexer and demultiplexer implementation.
7. Adder chips (half-adder, full-adder and incrementer).
8. Arithmetic logic unit implementation.
9. Combinational vs. sequential chips and quartz clocks.
10. Latches: S-R latches, data flip-flops (D latches).
11. 1-bit and n-bit register construction.
12. Recursive construction of RAM modules using 16-bit registers and multiplexers/demultiplexers for addressing individual registers.
13. Counter chip implementation (for the program counter register).
14. Computer architecture: defining the Hack instruction set.
15. Memory-mapped I/O for interfacing with peripherals.
16. CPU design and implementation.
17. Fitting everything into the Von Neumann architecture to complete the general-purpose computer.

## Hack Assembler

Assemblers are a special class of compilers that convert source code written in 
an assembly language into machine code that is directly executable by the CPU.

## Jack Virtual Machine & Compilation Model

See the Jack language specification and JACK VM language provided by Nand2Tetris ([CSIE slides](https://www.csie.ntu.edu.tw/~cyy/courses/introCS/13fall/lectures/handouts/lec11_Jack.pdf)).

In this project, we implement a C++ compiler based on a 2-tier compilation model
that's similar to the standard compilation of high-level languages like Java
and C#.

Jack is an [LL(1)](https://en.wikipedia.org/wiki/LL_parser) 'object-oriented' programming language. The Jack compilation model works in two stages, the first involves using the Jack
compiler to generate Jack VM code, which can then be run by the Jack Virtual Machine.
This idea is inspired by Java's compilation model where a Java compiler converts
Java source code to [*bytecode*](https://en.wikipedia.org/wiki/Java_bytecode) which can be 
executed by the [Java Virtual Machine](https://en.wikipedia.org/wiki/Java_virtual_machine). The the VM code is then mapped to the hardware platform's assembly language, then finally converted to machine code by
the assembler.

## Jack VM Translator

The VM translator is responsive for taking input written in the Jack VM language
and mapping it to Hack assembly language, which can be passed through the Hack
assembler to obtain executable machine code.

## Jack Compiler

The Jack compiler, developed with C++, CMake and GoogleTest, is responsible for
compiling files written in the Jack programming language into the Jack VM
language, which can be passed to the Jack VM translator to ultimately obtain 
executable machine code.

```bash
# Build and run.
cmake -S . -b build   # Generate the cross-platform build system (make sure to install CMake).
cmake --build build   # Produce build files.

./build/JackCompiler <file>        # Translates a single .jack file.
./build/JackCompiler <directory>   # Translates all .jack files in the given directory.
./build/JackCompiler               # Same as the above, but uses the current directory.
```

By default, `JackCompiler` outputs VM code to files along the same path as the 
input `.jack` source files.

To run tests for the compiler:

```bash
# Run unit tests.
cmake --build build
cd build && ctest --verbose   # The --verbose option outputs GoogleTest's test failure messages.

# Run integration tests.
sh SyntaxAnalysisTest.sh
sh CodeGenerationTest.sh

# ... or:
chmod +x SyntaxAnalysisTest.sh
chmod +x CodeGenerationTest.sh
./SyntaxAnalysisTest.sh
./CodeGenerationTest.sh
```

## Dev Notes: Compiler Lexical Analysis

A recount of how I implemented the compiler (part I). The major components are:
* `JackCompiler`
  The entry point of the compiler. It's responsible for handling command-line
  arguments and kicking off the compilation process starting from lexical
  analysis, building up an abstract syntax tree, then code generation.
* `CompilationEngine`
  A [recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser#:~:text=In%20computer%20science%2C%20a%20recursive, the%20nonterminals%20of%20the%20grammar.)
  that traverses the token stream produced by `LexicalAnalyser` and applies the
  rules of Jack grammar to compile language constructs like classes, 
  subroutines, statements and expressions.
* `LexicalAnalyser`
  This module is responsible for traversing the source code input stream and
  mapping it to a token stream that the CompilationEngine can extract from
  and process tokens.

Jack is almost an [LL(1)](https://en.wikipedia.org/wiki/LL_grammar) language that's
designed such that the parser would 'know' what the compilation target for each
token it encounters is without having to perform a look-ahead on the token
stream. The only exception to this is in expression parsing.

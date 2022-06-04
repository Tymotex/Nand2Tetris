# Jack Compiler

A Jack compiler, written in C++. Jack is an LL(1) object-oriented language.
The Jack compilation model works in two stages, the first involves using this
compiler binary to generate VM code that can be run by the Jack VM (in the same
fashion that Java compilers convert Java source to bytecode which can be 
executed by the JVM), then the VM is mapped to the hardware platform's assembly
language, then finally to machine code by the assembler.

```bash
# Build and run
cmake -S . -b build   # Generate the cross-platform build system (make sure to install CMake).
cmake --build build   # Produce build files

./build/JackCompiler <file>        # Translates a single .jack file.
./build/JackCompiler <directory>   # Translates all .jack files in the given directory.
./build/JackCompiler               # Same as the above, but uses the current directory.
```

By default, `JackCompiler` outputs VM code to files along the same path as the 
input `.jack` source files.

To run tests for the compiler:

```bash
# Run integration tests.
sh SyntaxAnalysisTest.sh

# ... or
chmod +x SyntaxAnalysisTest.sh
./SyntaxAnalysisTest.sh

# Run unit tests.
cmake --build build
cd build && ctest --verbose   # The --verbose option outputs GoogleTest's test failure messages.
```

Lexical analysis: all tokens are either *keywords*, *symbols*, *integer literals*, *string literals* or *identifiers*.

Grammar: 
* Defines the expected form for constructs like let statements, if-statements, while loops, etc.

### Plan:

LexicalAnalyser
* Finish this first
* JackCompiler loops through this, then for the current token decides which
  method of CompilationEngine to call.
  + CompilationEngine also has a reference to the SAME LexicalAnalyser and will continue
    to advance it forward in order to do recursive descent.

JackCompiler
* This will contain the while(has more tokens) advance, and will have a switch-case
  stack that determines what CompilationEngine method to call.
CompilationEngine
* Recursive descent.

### Implementation Ideas:

// TODO: bring these ideas to the documentation
* Remove has_more_tokens. Move this responsibility to advance(), which throws a
  custom exception for Jack Syntax error and returns true/false if there are
  any more tokens to parse.
* advance will scan forward and stop at the start of the next token. In doing so, 
  it'll also populate the fields of LexicalAnalyser, making it trivial to 
  implement `keyword` , `symbol` , etc.
  + Add more fields to LexicalAnalyser
  + The scanning forward part might require matching against various regex
    rules. This is the hard part I think. Check online if regex works well.

    - Wait, I'm confusing myself. This is compileXXX's job. All LexicalAnalyser
      needs to do is tokenise. This is an easier challenge.

    - There's probably no need to do this.
* Try to get `test-files` working with `ctest`.
* The CompilationEngine should probably have a delegator function that has a big switch-case
  stack that decides which compileXXX command to call. Ie. this should be placed
  in CompilationEngine instead of JackCompiler, since it's needed there.
* Have another read over 10.3 after getting something simple done Eg. Main.jack.

2. Implement tokeniser and ensure you pass all 3 tests for *T.xml
  - Write a bash test script to drive this. It runs ./JackCompiler and applies
    diffs against cmp files.
3. Get HelloWorld/Main.jack to produce correct Main.xml (your simple example of )

#### Lexical Analayser

Preprocess:
* Strip all comments: //, /* */

Identify which lexical element (class, constructor, {, 123, ...) the cursor is
currently pointing at. Skip all whitespace.
* If it starts with a number, then it must be INT_CONST.
  + Caveat: what if there is no whitespace after the number?
- 

TODO: for the future:
- Add for-loops to the language.
- Make it optional to output *T.xml
- Write up an explanation of how you implemented the compiler and what data structures you used and how your tokenisation and parsing algorithms work.
* There should be logic preventing multiple classes per file and nested classes.
* Disallow nested subroutines.
* Disallow integer constant overflow/underflow.
- 
* Investigate possiblity of tweaking the VM translator to produce valid x86 assembly code.
  Can leverage other intermediate language representations? Or other compilers?
* After completing everything, write a big blog summarising very concisely how
  to build a computer and programming language + compiler from first principles.

# How it works
A recount of how I implemented the compiler (part I)

The major components are:
- `JackCompiler`
  The entry point of the compiler. It's responsible for handling command-line
  arguments and kicking off the compilation process starting from lexical
  analysis, building up an abstract syntax tree, then code generation.
- `CompilationEngine`
  A [recursive descent parser](https://en.wikipedia.org/wiki/Recursive_descent_parser#:~:text=In%20computer%20science%2C%20a%20recursive,the%20nonterminals%20of%20the%20grammar.)
  that traverses the token stream produced by `LexicalAnalyser` and applies the
  rules of Jack grammar to compile language constructs like classes,
  subroutines, statements and expressions.
- `LexicalAnalyser`
  This module is responsible for traversing the source code input stream and
  mapping it to a token stream that the CompilationEngine can extract from
  and process tokens.

Jack is almost an [LL(1)](https://en.wikipedia.org/wiki/LL_grammar) language that's
designed such that the parser would 'know' what the compilation target for each
token it encounters is without having to perform a look-ahead on the token
stream. The only exception to this is in expression parsing.

# Compiler II

### Scope
Every identifier is implicitly associated with a scope.
- Static and field variables are meant to be scoped to the class only.
  - Note: the class-level symbol table gives enough information for the OS to
    know how much memory to allocate to objects of the class.
- Local and argument variables are scoped to the subroutine only.

We use 2 static symbol tables, one for tracking class-level scope and one for
tracking the subroutine-level scope. When the compiler wants to verify that a
symbol is in scope, it checks the closest scope first, then proceeds to outward
scopes. A variable is considered not in scope if no scope 'layer' contains that
variable in the symbol table.

In Jack, we can only support 2 scope layers. To support arbitrary levels of scope nesting, we'd have to use a linked list
structure with each node containing a symbol table, for example.

When inside a class declaration, a new class-level symbol table is created and
any instance/static field declarations insert a new entry to the table.
Likewise, when encoutering a subroutine declaration, the compile creates a new
subroutine-level symbol table and inserts new entries for any local variable
declaration.

# Methods

- Methods always get `this` as their first argument implicitly.

TODO: "If no path is specified, the compiler operates on the current folder."

# Implementation plan:

- Each .jack file produces a corresponding .vm file with the same name.
  Eg. X.jack compiles to X.vm
- Subroutine f in X.jack becomes a VM function with name `X.f`.
- Each of Foo's static variables gets mapped to `static 0`, `static 1`, `static 2`
- Each of Foo's field variables gets mapped to `this 0`, `this 1`, `this 2`
- Each of a constructor or function's arguments gets mapped to `argument 0`, `argument 1`, `argument 2`
- Each of a method's arguments variables gets mapped to `argument 1`, `argument 2`, `argument 3`. This starts from 1!!!
- Each of a constr/func/meth's local variables gets mapped to `local 0`, `local 1`, `local 2`
- When an object method is referenced:
    push argument 0   # To get `this`' memory address onto the stack.
    pop pointer 0     # To put it into the THIS RAM register.
    The caller is responsible for putting this into argument 0.
- Arrays (re-read those pages)
- null and false become `push constant 0`. 
- true becomes `push constant 1` followed by `neg` to make it 1111111111111111 in memory.
- this becomes `push pointer 0`

- Handling identifiers:
  - Any identifier not in the symbol table can be assumed to be a class name or subroutine name (for valid code only).
  - There is no linking anyway. There's no need to keep subroutines and class names in the symbol table.
- Compiling expressions:
  - Implement `codeWrite`
  - * and / become `push operand1`, `push operand2` then `call Math.multiply 2` or `call Math.divide 2`
- Compiling strings:
  - Push strlen, then `call String.new`, then push each character c onto the stack and calling appendChar for each of them.
  - Remember, appendChar returns the string, and so does the constructor.
- Compiling func and constr *calls*:
  - First, invoke compileExpressionList to get the n args onto the stack, then `call ___ n`
- Compiling method *calls*.
  - First, push the mem address of the object which is being acted on onto the stack for argument 0,
  - Then, invoke compileExpressionList to get the subsequent n args onto the stack, then `call ___ n`.
- Compiling do statements:
  - Just compile it as if it were `do expression`
  - Discard the value by `pop temp 0`. All functions will return something. Void functions will do `push constant 0`, `return` by convention.
- Compiling classes:
  - Make a symbol table, then add to it all the field and static variables.
  - Also make an empty subroutine symbol table.
- Compiling subroutines:
  - if method: Make a new subroutine symbol table, then insert <this, className, arg, 0>
  - insert all parameters into table
  - insert all local variables into table
  - `function className.subroutineName nVars`, with nVars being the num of local variables.
  - if method: then `push argument 0`, `pop pointer 0`
  - if void: then `push constant 0` and `return`. The caller will discard the 0 by convention.
- Compiling constructors:
  - All of the above, but then do: `push constant nFields`, `call Memory.alloc 1`, `pop pointer 0`, which sets THIS to point to a new obj in the heap.
  - `push pointer 0`, `return`. 
- Compiling arrays:
  - Just follow the page on array compilation.

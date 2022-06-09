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
  + Write a bash test script to drive this. It runs ./JackCompiler and applies
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
* `./build/JackCompiler non-existent` fails
* Add for-loops to the language.
* Add else-if or elif to the language.
* Instead of `function`, have `static method` instead.
* Support or catch & disallow the case of accessing a field of an object directly, eg. `foo.bar`.
* Check return types match the subroutine signature.
* Make it so that you don't have to `return this;`
* CLI argument processing. Make it optionally output XML.
  * Make it optional to output *T.xml
* Write up an explanation of how you implemented the compiler and what data structures you used and how your tokenisation and parsing algorithms work.
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

# Compiler II

### Scope

Every identifier is implicitly associated with a scope.
* Static and field variables are meant to be scoped to the class only.
  + Note: the class-level symbol table gives enough information for the OS to
    know how much memory to allocate to objects of the class.

* Local and argument variables are scoped to the subroutine only.

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


# Symbol Table
* Whenever an identifier is encountered, we look it up in the symbol table.
* Separate namespaces/scopes are achieved by simply managing two symbol tables.
  One for the class fields, one for local args/vars. If x appears in both tables, 
  then prioritise the inner scope (local) over the outer (class). If x appears
  in neither, then it's assumed to be a subroutine name or class name.
* When we see a new `static` or `field`, insert new row to the class table. When
  we see a new `var` or local argument, we'd insert it into the subroutine
  table.

# Expressions
`(x + y) * 2` becomes:
- push x
- push y
- add
- push 2
- multiply

Use the algo:
```cpp
void code_write(expression) {
  if (expression is a single-term constant) out << "push c";
  if (expression is a single-term var) out << "push var";
  if (expression is a multi-term) {
    code_write(first_term);
    code_write(rest_of_expr);
  }
  if (expression has unary-operator) {
    code_write(expressions);
    out << "op";
  }
  if (expression is subroutine) {
    for each expr in expressionlist:
      code_write(each_expr)
    out << "call f n"
  }
}
```

# Strings
Strings are implemented as a class by the OS. The compiler simply maps string
literals to invocations of String.new.
Yup, there is a memory leak since we don't have a system to reclaim the
memory...

Consider `"hello world"`.
1. Push the string length first, in this case 11: `push constant 11`.
2. Call the OS string constructor: `call String.new 1`.
3. For each character `c` of `"hello world"`, do:
  - `push constant c`
  - `call String.append 1` (note that the string is returned by append, so there's no need to repush `this`.)

# Statements

- `return` - first call compileExpression, then `return`. The VM code supports function calls, remember?
- `let varName = expression` - first call compileExpression, then pop it into varName `pop varName` where varName is a segment + index like local 0, static 2, etc.
- `do subroutineCall` - first call compileSubroutineInvocation, then `pop temp 0`. Remember, do will not utilise the return value, so you can just discard it with `pop temp 0`.
- `if (expression) { statements } else { statements }` - first call compileExpression, then `not`, then an `if-goto ELSE_LABEL` that tests that expression's value against 0.
  - Each label must have a running counter that uniquely identifies it. 
  - Remember, compileStatements is recursive, so you can recursively append a running counter.
  ```
    if (expression) {
      statements1
    } else {
      statements2
    }

    // becomes
      compiledExpression 
      not
      if-goto ELSE_LABEL  // If the expression was true, then this conditional goto does nothing.
      compiledStatements1 
      goto END_IF
    label ELSE_LABEL:
      compiledStatements2
    label END_IF
  ```
  ```
    
  ```
- `while (expression) { statements }` - very similar to how to compile if
  ```
    label WHILE_START
      compiledExpression
      not
      if-goto WHILE_END
      compiledStatements
      goto WHILE_START
    label WHILE_END
  ```

# Objects

Remember, each variable that is an object is holding the memory address to the
base of the object in the heap.

When we invoke a method on a variable `foo` holding the base memory address of an 
object on the heap, like `foo.bar()`, we `push foo` and `pop pointer 0`

# Constructors

`let p = Point.new(2, 3)`

Remember, a constructor call is just a regular subroutine invocation. We'd do
`push 2`, `push 3`, `call Point.new 2` and expect the address of the new
instance. We handle the invocation of static methods (functions) the same way.

The magic happens in the compilation of the constructor body.
1. First, we call an OS function to allocate memory, then initialise fields. We
know the size of the memory block to allocate based on the class-level symbol
table. Eg. here we know that the Point class has 2 int fields, so we can do:
`push constant 2`, `call Memory.alloc 1` which puts the address of a memory
block with 2 16-bit slots onto the stack.

2. Set the address given by the memory alloc call as `this`. To accomplish that,
we simple do `pop pointer 0`.

3. compileStatements

4. Return the address of `this` by doing: `push pointer 0` and `return`.

# Methods

Consider `p1.distance(p2)`. The compiler handles these method invocations
*as if* they were procedural function calls like `distance(p1, p2)`, which
would map easily to VM code: `push p1`, `push p2`, `call distance 2`.

### Invocation
Note: there are 2 forms of method invocations
1. a) On a variable: `p1.distance(p2)`. Here we `push p1` from the symbol table.
   b) On the current instance: `distance(p2)`, which is the same as `this.distance(p2)`. Here, we push whatever is in this: `push this`.
2. compileExpressionList, which leaves all n arguments on the stack
3. `call className.methodName n + 1` (remember, the first arg was the address we wanted the callee to have as `this`).
  - We source `className` from the `type` column of the symbol table.

### Callee
1. Align `this` by doing: `push argument 0` then `pop pointer 0`.
2. Compile the rest of the statements.

# Arrays

Consider `let x = arr[i]`. Remember, arr is variable that holds the base address
of an object on the heap. We can access `arr[i]` by simply doing: `base address of arr + i`,
ie. the VM code: `push arr`, `push i`, `add`. Now, we do `pop pointer 1` to 
align `THAT`, followed by `push that 0`, `pop x`.

Consider `let a[expr1] = expr2`. 
1. We first put `a[expr1]` onto the stack.
2. Then, we put `expr2` on the stack by doing: compileExpression(expr2) and
`pop temp 0`.
3. Now, we `pop pointer 1`, `push temp 0`, `pop that 0` to write `expr2` into
`a[expr1]`.

We can now handle something like `a[b[i] + a[j + b[a[3]]]] = b[b[j] + 2]`.

# Implementation plan:
* Each .jack file produces a corresponding .vm file with the same name.
  Eg. X.jack compiles to X.vm
* Subroutine f in X.jack becomes a VM function with name `X.f`.
* Each of Foo's static variables gets mapped to `static 0`,       `static 1`,       `static 2`
* Each of Foo's field variables gets mapped to `this 0`,       `this 1`,       `this 2`
* Each of a constructor or function's arguments gets mapped to `argument 0`,       `argument 1`,       `argument 2`
* Each of a method's arguments variables gets mapped to `argument 1`,       `argument 2`,       `argument 3`. This starts from 1!!!
* Each of a constr/func/meth's local variables gets mapped to `local 0`,       `local 1`,       `local 2`
* When an object method is referenced:
    push argument 0   # To get `this` ' memory address onto the stack.
    pop pointer 0     # To put it into the THIS RAM register.
    The caller is responsible for putting this into argument 0.

* Arrays (re-read those pages)
* null and false become `push constant 0`. 
* true becomes `push constant 1` followed by `neg` to make it 1111111111111111 in memory.
* this becomes `push pointer 0`

* Handling identifiers:
  + Any identifier not in the symbol table can be assumed to be a class name or subroutine name (for valid code only).
  + There is no linking anyway. There's no need to keep subroutines and class names in the symbol table.
* Compiling expressions:
  + Implement `codeWrite`
  + * and / become `push operand1`,  `push operand2` then `call Math.multiply 2` or `call Math.divide 2`
* Compiling strings:
  + Push strlen, then `call String.new`, then push each character c onto the stack and calling appendChar for each of them.
  + Remember, appendChar returns the string, and so does the constructor.
* Compiling func and constr *calls*:
  + First, invoke compileExpressionList to get the n args onto the stack, then `call ___ n`
* Compiling method *calls*.
  + First, push the mem address of the object which is being acted on onto the stack for argument 0, 
  + Then, invoke compileExpressionList to get the subsequent n args onto the stack, then `call ___ n`.
* Compiling do statements:
  + Just compile it as if it were `do expression`
  + Discard the value by `pop temp 0`. All functions will return something. Void functions will do `push constant 0`,  `return` by convention.
* Compiling classes:
  + Make a symbol table, then add to it all the field and static variables.
  + Also make an empty subroutine symbol table.
* Compiling subroutines:
  + if method: Make a new subroutine symbol table, then insert <this, className, arg, 0>
  + insert all parameters into table
  + insert all local variables into table
  + `function className.subroutineName nVars`, with nVars being the num of local variables.
  + if method: then `push argument 0`,       `pop pointer 0`
  + if void: then `push constant 0` and `return`. The caller will discard the 0 by convention.
* Compiling constructors:
  + All of the above, but then do: `push constant nFields`,       `call Memory.alloc 1`,       `pop pointer 0`, which sets THIS to point to a new obj in the heap.
  + `push pointer 0`,  `return`. 
* Compiling arrays:
  + Just follow the page on array compilation.

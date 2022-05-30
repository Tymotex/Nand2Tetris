# Jack Compiler

Written in C++.

Lexical analysis: all tokens are either *keywords*, *symbols*, *integer literals*, *string literals* or *identifiers*.

Grammar: 
- Defines the expected form for constructs like let statements, if-statements, while loops, etc.

Plan:
LexicalAnalyser
- Finish this first
- JackCompiler loops through this, then for the current token decides which
  method of Parser to call.
  - Parser also has a reference to the SAME LexicalAnalyser and will continue
    to advance it forward in order to do recursive descent.
JackCompiler
- This will contain the while(has more tokens) advance, and will have a switch-case
  stack that determines what Parser method to call.
Parser
- Recursive descent.

TODO: for the future:
- Investigate possiblity of tweaking the VM translator to produce valid x86 assembly code.

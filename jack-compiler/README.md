# Jack Compiler

Written in C++.
Jack is an LL(1), context-free language.

Lexical analysis: all tokens are either *keywords*, *symbols*, *integer literals*, *string literals* or *identifiers*.

Grammar: 
* Defines the expected form for constructs like let statements, if-statements, while loops, etc.

### Plan:

LexicalAnalyser
* Finish this first
* JackCompiler loops through this, then for the current token decides which
  method of Parser to call.
  + Parser also has a reference to the SAME LexicalAnalyser and will continue
    to advance it forward in order to do recursive descent.

JackCompiler
* This will contain the while(has more tokens) advance, and will have a switch-case
  stack that determines what Parser method to call.
Parser
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
* The Parser should probably have a delegator function that has a big switch-case
  stack that decides which compileXXX command to call. Ie. this should be placed
  in Parser instead of JackCompiler, since it's needed there.
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
- Make it optional to output *T.xml
- Write up an explanation of how you implemented the compiler and what data structures you used and how your tokenisation and parsing algorithms work.
* There should be logic preventing multiple classes per file and nested classes.
* Disallow nested subroutines.
* Disallow integer constant overflow/underflow.
- 
* Investigate possiblity of tweaking the VM translator to produce valid x86 assembly code.

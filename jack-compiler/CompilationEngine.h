// Top-down parser that acts on a token stream to produce a parse tree.
#ifndef PARSER_H
#define PARSER_H

#include "LexicalAnalyser.h"
#include "SymbolTable.h"
#include <ostream>
#include <fstream>
#include <memory>
#include <string>

class CompilationEngine {
public:
    /**
     * Takes in the token stream produced by LexicalAnalyser and the output
     * file stream. The parser advances through the token stream with 
     * recursive descent.
     */
    explicit CompilationEngine(std::shared_ptr<LexicalAnalyser> lexical_analyser,
        std::ostream& vm_stream, std::ostream& xml_stream);

    ~CompilationEngine();

    /**
     * Compiles a class construct.
     */
    void compile_class();

    /**
     * Compiles the class' body.
     * This method should be invoked by `compile_class` after the lexical
     * analyser's cursor has been progressed beyond the class' opening { symbol.
     */
    void compile_class_body();

    /**
     * Compiles instance variables and static variables declared in a class
     * body.
     */
    void compile_class_field_declaration();

    /**
     * Compiles a complete instance method, static method or constructor.
     */
    void compile_subroutine();

    /**
     * Compiles the parameter list defined by the subroutine signature.
     */
    void compile_parameter_list();

    /**
     * Compiles the statements inside the subroutine's body.
     */
    void compile_subroutine_body();

    /**
     * Compiles a standard local variable declaration with `var`.
     */
    void compile_variable_declaration();

    /**
     * Compiles a sequence of regular statements in the current { } scope.
     */
    void compile_statements();

    /**
     * Compiles a `let` statement.
     */
    void compile_let();

    /**
     * Compiles an `if` statement and its trailing `else` statement if it
     * exists.
     */
    void compile_if();

    /**
     * Compiles a `while` loop.
     */
    void compile_while();

    /**
     * Compiles a `do` statement subroutine invocation.
     */
    void compile_do();

    /**
     * Compiles a `return` statement.
     */
    void compile_return();

    /**
     * Compiles an expression which may consist of constants, literals,
     * variables, invocations, symbols, etc.
     */
    void compile_expression(int nest_level);

    /**
     * Compiles an individual *term*, which appears as tokens within an 
     * *expression*.
     */
    void compile_term(int nest_level);

    /**
     * Compiles a subroutine invocation.
     */
    void compile_subroutine_invocation();

    /**
     * Compiles a comma-separated list of *expressions*.
     */
    int compile_expression_list();

    /**
     * Attempts to compile an array access. If it fails to compile, then return
     * false.
     */
    bool try_compile_subscript();

    /**
     * Attempts to compile a comma-separate list of variables. If it fails, then
     * return false.
     */
    bool try_compile_trailing_variable_list();
private:
    /**
     * Handle on the token stream producer, ie. the Jack lexical analyser.
     */
    std::shared_ptr<LexicalAnalyser> _lexical_analyser;

    /**
     * An XML writer that CompilationEngine writes the parse tree into. Useful for testing
     * and debugging.
     */
    std::unique_ptr<XMLOutput> _xml_parse_tree;

    /**
     * Output stream that VM instructions get written to.
     */
    std::ostream& _vm_out;

    /**
     * Class-level symbol table storing all identifiers declared with `static`
     * and `field`.
     */
    SymbolTable _class_symbol_table;

    /**
     * Subroutine-level symbol table storing all identifiers declared with
     * `var` or received as an argument.
     */
    SymbolTable _subroutine_symbol_table;

    /**
     * Throws a JackParseException if the current token is of a certain type, 
     * equal to a certain string, or is a certain keyword, etc.
     * Invoking these functions will always advance the token stream cursor
     * forward one step AND capture the token as XML.
     */
    std::string expect_token_type(TokenType token_type, const std::string& err_message);
    std::string expect_token(const std::string& token, const std::string& err_message);

    // Expects one of: primitive types, built-in classes or user-defined
    // classes.
    std::string expect_data_type(const std::string& err_message);   

    /**
     * Dumps the current token to the XML stream.
     */
    std::string xml_capture_token();
};

class JackCompilationEngineError : public std::exception {
public:
    static const size_t MAX_MSG_LEN = 64;

    JackCompilationEngineError(const std::string& message) throw();
    JackCompilationEngineError(LexicalAnalyser& lexical_analyser, char const* const message) throw();
    JackCompilationEngineError(LexicalAnalyser& lexical_analyser, const std::string& message) throw();

    virtual char const* what() const throw();
private:
    char const* _message;
};

#endif
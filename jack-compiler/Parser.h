// Top-down parser that acts on a token stream to produce a parse tree.
#ifndef PARSER_H
#define PARSER_H

#include "LexicalAnalyser.h"
#include <fstream>
#include <memory>
#include <string>

class Parser {
public:
    /**
     * Takes in the token stream produced by LexicalAnalyser and the output
     * file stream. The parser advances through the token stream with 
     * recursive descent.
     */
    explicit Parser(std::shared_ptr<LexicalAnalyser> lexical_analyser, std::ofstream& o);

    /**
     * Compiles a class construct.
     */
    void compile_class();

    // TODO: remove this?
    // /**
    //  * Compiles instance variables and static variables declared in a class
    //  * body.
    //  */
    // void compile_member_variable_declaration();

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
    void compile_expression();

    /**
     * Compiles an individual *term*, which appears as tokens within an 
     * *expression*.
     */
    void compile_term();

    /**
     * Compiles a comma-separated list of *expressions*.
     */
    int compile_expression_list();

private:
    std::shared_ptr<LexicalAnalyser> _lexical_analyser;

    /**
     * Compiles the class' body.
     * This method should be invoked by `compile_class` after the lexical
     * analyser's cursor has been progressed beyond the class' opening { symbol.
     */
    void compile_class_body();
};

#endif
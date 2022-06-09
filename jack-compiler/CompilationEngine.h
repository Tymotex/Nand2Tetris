// Top-down parser that acts on a token stream to produce a parse tree.
#ifndef PARSER_H
#define PARSER_H

#include "LexicalAnalyser.h"
#include "SymbolTable.h"
#include "VMWriter.h"
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
        const std::string& translation_unit_name, std::ostream& vm_stream,
        std::ostream& xml_stream);

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
     * Using the given prefix, recursively constructs the fully qualified
     * subroutine name.
     */
    void compile_subroutine_invocation(const std::string& subroutine_name_prefix);

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
    bool try_compile_trailing_variable_list(const std::string data_type,
        const std::string& decl_type, const bool is_subroutine_scope);

    /**
     * Recursive helper to resolve arbitrary depth of '.' chaining in subroutine
     * invocation.
     * Eg. Compiles `foo.bar.baz()`.
     */
    void compile_subroutine_invocation_recursive(
        const std::string& first_token, const std::string& subroutine_name_prefix, int depth);
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
     * Name of the class. Used to namespace class members such as fields and
     * subroutines.
     */
    std::string _class_name;

    /**
     * The module responsible for writing VM instructions.
     * Contains the output stream that VM instructions get written to.
     */
    VMWriter _vm_writer;

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
     * A running counter that is embedded into labels to uniquely identify them.
     */
    int _uniq_counter_if;
    int _uniq_counter_while;

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

    /**
     * Searches through the class-level and subroutine-level symbol tables for
     * the given identifier.
     */
    bool is_identifier_defined(const std::string& identifier);

    /**
     * Locates the deepest symbol table that hosts the given identifier.
     * Assumes that the given identifier exists in at least one symbol table.
     */
    SymbolTable& get_symbol_table_containing(const std::string& identifier);

    /**
     * Maps the declaration type to the corresponding virtual memory segment 
     * that we expect to store the declared identifier.
     * Eg. if we declared `var int x;`, then `decl_type_to_segment(DeclarationType::VAR)`
     *     would map to `VirtualMemorySegment::LOCAL`. This ultimately informs 
     *     the compiler where to read/write to `x` in RAM.
     */
    VirtualMemorySegment decl_type_to_segment(const DeclarationType decl_type);

    void construct_string(const std::string& s);
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
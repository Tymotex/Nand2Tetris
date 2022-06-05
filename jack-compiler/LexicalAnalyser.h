// Performs lexical analysis on Jack source code, ie. maps a character stream
// to a token stream that's later used for syntactic analysis.
#ifndef LEXICAL_ANALYSER_H
#define LEXICAL_ANALYSER_H

#include "utils/XMLOutput.h"
#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>
#include <regex>
#include <tuple>
#include <memory>

enum class TokenType {
    KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST, COMMENT, UNDEFINED
};

enum class Keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC,
    FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NULL_KEYWORD, THIS,
    UNDEFINED
};

class LexicalAnalyser {
public:
    // Contains all the keywords defined by the Jack language standard.
    // Eg. class, method, function, constructor, var, let, etc.
    static std::unordered_set<std::string> general_keywords;
    static std::unordered_set<std::string> statement_keywords;
    static std::unordered_set<std::string> subroutine_declarers;
    static std::unordered_set<std::string> data_types;
    static std::unordered_set<std::string> field_declarers;
    static std::unordered_set<std::string> builtin_literals;

    // Contains all the symbols defined by the Jack language standard.
    // For example: ,.;(){}*|&~<>= and so on.
    static std::unordered_set<std::string> general_symbols;
    static std::unordered_set<std::string> binary_operators;
    static std::unordered_set<std::string> unary_operators;

    // Maps a stringified keyword to a keyword enum.
    // Eg. "class" to Keyword::CLASS.
    static std::unordered_map<std::string, Keyword> string_to_keyword;

    // Stringifies a TokenType enum value.
    // Eg. TokenType::KEYWORD is mapped to "keyword".
    static std::unordered_map<TokenType, std::string> token_type_to_string;

    static std::regex valid_identifier_pattern;

    /**
     * Opens an input file stream from the given Jack source. Prepares it for
     * tokenisation.
     */
    explicit LexicalAnalyser(std::istream& jack_stream, std::ostream& token_xml_stream);

    ~LexicalAnalyser();

    /**
     * Advances the cursor forward through the Jack source stream and returns
     * true if it was able to produce a token from the character stream.
     * Throws `JackSyntaxError` if an invalid token is encountered.
     */
    bool try_advance();

    /**
     * Returns the TokenType of the token that's currently being pointed to by
     * the stream cursor.
     */
    TokenType token_type();

    /**
     * Returns the Keyword of the current KEYWORD token. Assumes that this 
     * method is invoked only when the current token is of the keyword type.
     */
    Keyword keyword();

    /**
     * Returns the character corresponding to the current symbol token. Assumes
     * that this method is invoked only when the current token is of the SYMBOL
     * type.
     */
    char symbol();

    /**
     * Returns the indentifier's name as a string. Assumes that this method is
     * invoked only when the current token is of the IDENTIFIER type.
     */
    std::string identifier();

    /**
     * Returns the integer value of the current token. Assumes that this method
     * is invoked only when the current token is of the INT_CONST type.
     */
    int get_int_value();

    /**
     * Returns the string value of the current token. Assumes that this method
     * is invoked only when the current token is of the STRING_CONST type.
     */
    std::string get_token();

    /**
     * Look ahead at the next token in the stream, if it exists.
     */
    std::string peek();

    /**
     * Seeks one byte backwards in the input stream.
     */
    void step_back();

    /**
     * Restarts the lexical analysis, seeking back to the start of the input
     * stream.
     */
    void reset();

    /**
     * Produces a token stream and writes it as XML to the designated output
     * stream. Resets the analyser.
     */
    void write_xml_tokens(const bool& enable_debug);

    /**
     * Advances until the first (and only) class declaration is reached. At the
     * end of the invocation, the stream cursor should point to the `class`
     * token.
     */
    bool try_advance_until_class_declaration();

    /**
     * Determines whether or not the given token is a keyword.
     */
    bool is_keyword(const std::string& token);

    /**
     * Determines whether or not the given token is a symbol.
     */
    bool is_symbol(const std::string& token);

    // Returns the enum value for the given keyword string.
    Keyword get_keyword(const std::string& keyword);
    
    // Returns the stringified token type.
    std::string get_token_type();

    /**
     * Determines whether the given identifier follows identifier naming rules.
     */
    static bool is_valid_identifier(const std::string& identifier);

private:
    /**
     * Jack character input stream.
     */
    std::istream& _jack_in;

    /**
     * Token XML output stream.
     */
    std::ostream& _token_xml_out;

    // Cursor helper variables.
    std::string _curr_token;
    TokenType _curr_token_type;
    Keyword _curr_keyword;

    /**
     * Dumps debug info to `stdout`.
     */
    void show_tokeniser_debug_info();

    /**
     * Advances the cursor until any non-whitespace character is encountered.
     */
    void try_advance_past_whitespace();

    /**
     * Attempts to read a string literal, stopping until a " is encountered,
     * before a newline character is encountered.
     * Reads forward from where the cursor's current position and advances it
     * forward as a side effect.
     */
    void try_read_string_literal();

    /**
     * Attempts to read a int literal. 
     * Reads forward from where the cursor's current position and advances it
     * forward as a side effect.
     */
    void try_read_int_literal();

    /**
     * Attempts to advance just beyond the end of the inline or multi-line
     * comment. Returns whether or not a valid comment was advanced past.
     */
    bool try_advance_past_comment(const bool& multiline);

    /**
     * Attempts to read a Jack keyword, returning whether the attempt succeeded.
     * Reads forward from where the cursor's current position and advances it
     * forward as a side effect.
     */
    bool try_read_keyword();

    /**
     * Attempts to read an identifier. Throws JackSyntaxError if the identifier
     * violates naming rules.
     * Reads forward from where the cursor's current position and advances it
     * forward as a side effect.
     */
    void try_read_identifier();

    bool is_prefix_of_any_keyword(const std::string& token, std::unordered_set<std::string>& lexicon);
};

class JackSyntaxError : public std::exception {
public:
    static const size_t MAX_MSG_LEN = 64;

    JackSyntaxError(LexicalAnalyser& lexical_analyser, const std::string& message) throw();
    JackSyntaxError(LexicalAnalyser& lexical_analyser, char const* const message) throw();
    virtual char const* what() const throw();
private:
    char const* _message;
};

#endif
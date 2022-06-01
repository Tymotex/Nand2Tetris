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
    // Eg. type: {int, char, boolean},
    //     subroutine_declaration: {function, method, constructor},
    //     ... and so on
    static std::unordered_set<std::string> keyword_lexicon;

    // Contains all the single-character symbols defined by the Jack language
    // standard. For example: ,.;(){}*|&~<>= and so on.
    static std::unordered_set<char> symbol_lexicon;

    static std::regex valid_identifier_pattern;

    /**
     * Opens an input file stream from the given Jack source. Prepares it for
     * tokenisation.
     */
    explicit LexicalAnalyser(const std::string& source_jack_file_path,
        const std::string& token_xml_output_path);

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
    std::string get_str_value();

    /**
     * Seeks one byte backwards in the input stream.
     */
    void step_back();

    /**
     * Restarts the lexical analysis, seeking back to the start of the input
     * stream.
     */
    void reset();


    Keyword get_keyword(const std::string& keyword);

    std::string get_token_type();
private:
    /**
     * Jack character input stream.
     */
    std::ifstream _jack_in;

    /**
     * Output stream for all identified tokens. This is mainly for sanity
     * checking during compiler development.
     */
    std::unique_ptr<XMLOutput> _token_xml_out;
    std::string _token_xml_output_path;

    // Cursor helper variables.
    std::string _curr_token;
    TokenType _curr_token_type;
    Keyword _curr_keyword;

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
    // TODO: think of all the different cases for when to 'stop' reading a number.
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
};

// TODO: how to do exception classes *properly*
class JackSyntaxError : public std::exception {
public:
    static const size_t MAX_MSG_LEN = 64;

    JackSyntaxError(char const* const message) throw();
    virtual char const* what() const throw();
private:
    char const* _message;
};

#endif
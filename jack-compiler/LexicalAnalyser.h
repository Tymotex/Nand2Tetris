// Performs lexical analysis on Jack source code, ie. maps a character stream
// to a token stream that's later used for syntactic analysis.
#ifndef LEXICAL_ANALYSER_H
#define LEXICAL_ANALYSER_H
#include <string>

enum class TokenType {
    KEYWORD, SYMBOL, IDENTIFIER, INT_CONST, STRING_CONST
};

enum class Keyword {
    CLASS, METHOD, FUNCTION, CONSTRUCTOR, INT, BOOLEAN, CHAR, VOID, VAR, STATIC,
    FIELD, LET, DO, IF, ELSE, WHILE, RETURN, TRUE, FALSE, NULL_KEYWORD, THIS
};

class LexicalAnalyser {
public:
    /**
     * Opens an input file stream from the given Jack source. Prepares it for
     * tokenisation.
     */
    explicit LexicalAnalyser(const std::string& source_jack_file_path);

    /**
     * Determines whether the opened Jack source stream has any more tokens.
     */
    bool has_more_tokens();

    /**
     * Advances the cursor forward through the Jack source stream.
     */
    void advance();

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

private:
};

#endif
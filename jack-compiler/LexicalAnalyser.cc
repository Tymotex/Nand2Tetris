#include "LexicalAnalyser.h"
#include <string>
#include <iostream>
#include <unordered_set>
#include <regex>
#include <tuple>

std::unordered_set<std::string> LexicalAnalyser::keyword_lexicon = {
    // Class keywords:
    "class", "field", "static",
    // Data type keywords:
    "int", "char", "boolean",
    // Subroutine keywords:
    "constructor", "function", "method", "void",
    // Variable/value keywords:
    "var", "true", "false", "null", "this",
    // Statement keywords:
    "let", "do", "if", "else", "while", "return"
};

std::unordered_set<char> LexicalAnalyser::symbol_lexicon = {
    '{', '}', '(', ')', '[', ']', '.', ',', ';', '+', '-', '*', '/', '&', '|',
    '<', '>', '=', '~'
};

// TODO: why does $ not work in C++ regex? How can I get it working?
std::regex LexicalAnalyser::valid_identifier_pattern = std::regex(R"(^[a-zA-Z]\w+$)");

LexicalAnalyser::LexicalAnalyser(const std::string& source_jack_file_path,
    const std::string& token_xml_output_path)
    : _jack_in(std::ifstream(source_jack_file_path)),
      _token_xml_out(std::ofstream(token_xml_output_path)),
      _curr_token(""),
      _curr_token_type(TokenType::UNDEFINED),
      _curr_keyword(Keyword::UNDEFINED) {
    // Start the XML token stream.
    _token_xml_out << "<tokens>\n";
}

LexicalAnalyser::~LexicalAnalyser() {
    _token_xml_out << "</tokens>\n";
    _token_xml_out.close();
}

/**
 * In Jack, the first non-whitespace character encountered can
 * disambiguate a lot about what kind of token the source code stream
 * cursor is currently pointing to.
 * 
 * Eg. If the first char is ", then invoke `try_read_str`.
 *     If the first char is 0-9, then invoke `try_read_int`.
 *     If the first char is symbol, then done.
 *     If the first char is /, then 
 *         If the second char is /, then attempt to read an inline comment.
 *         Else if the second char is *, then attempt to read a multi-line comment.
 *         Else syntax error.
 *     Else
 *         First attempt to match the token against standard Jack keywords.
 *         If that fails, then attempt to read the token as an identifier.
 */
bool LexicalAnalyser::try_advance() {
    // Progress cursor until it reaches the first non-whitespace character.
    try_advance_past_whitespace();
    if (_jack_in.eof()) return false;

    char first_char = _jack_in.get();

    if (first_char == '"') {
        try_read_string_literal();
        _curr_token_type = TokenType::STRING_CONST;
    } else if (std::isdigit(first_char)) {
        try_read_int_literal();
        _curr_token_type = TokenType::INT_CONST;
    } else if (symbol_lexicon.find(first_char) != symbol_lexicon.end()) {
        _curr_token = std::string{first_char};
        _curr_token_type = TokenType::SYMBOL;
    } else if (first_char == '/') {
        char second_char = _jack_in.get();
        if (second_char == '/') {
            try_advance_past_comment(false);
        } else if (second_char == '*') {
            try_advance_past_comment(true);
        } else {
            throw JackSyntaxError("Invalid comment.");
        }
    } else {
        // Take one step back to read in the full token.
        _jack_in.seekg(-1, std::ios_base::cur);
        if (!try_read_keyword()) {
            try_read_identifier();
        }
    }

    // Write the extracted token to the debug info XML output stream.
    _token_xml_out << "<" << get_token_type() << "> "
                   << _curr_token
                   << " </" << get_token_type() << ">\n";

    return true;
}

TokenType LexicalAnalyser::token_type() {
    return _curr_token_type;
}

Keyword LexicalAnalyser::keyword() {
    return _curr_keyword;
}

char LexicalAnalyser::symbol() {
    return _curr_token[0];
}

std::string LexicalAnalyser::identifier() {
    return get_str_value();
}

int LexicalAnalyser::get_int_value() {
    return stoi(_curr_token);
}

std::string LexicalAnalyser::get_str_value() {
    return _curr_token;
}

void LexicalAnalyser::try_advance_past_whitespace() {
    while (!(_jack_in.eof())) {
        if (!std::isspace(_jack_in.get())) {
            if (_jack_in.eof()) return;
            _jack_in.seekg(-1, std::ios_base::cur);
            return;
        }
    }
}

void LexicalAnalyser::try_read_string_literal() {
    std::string token = "";
    char c;
    while ((c = _jack_in.get()) != '"') {
        if (_jack_in.eof() || c == '\n')
            throw JackSyntaxError("Expected \" before newline or EOF while reading string literal.");
        token.push_back(c);
    }
    _curr_token = token;
}

void LexicalAnalyser::try_read_int_literal() {

    // TODO: implement me.
}

bool LexicalAnalyser::try_advance_past_comment(const bool& multiline) {
    // TODO: implement me.

    _curr_token_type = TokenType::COMMENT;
    return false;
}

bool LexicalAnalyser::try_read_keyword() {
    std::string token = "";

    int num_read = 1;
    char c = _jack_in.get();
    token.push_back(c);

    // TODO: this should not be hard-coded... The 11 is meant to be strlen("constructor")
    while (num_read <= 11) {
        if (_jack_in.eof())
            throw JackSyntaxError("Unexpected EOF while reading Jack keyword.");
        if (keyword_lexicon.find(token) != keyword_lexicon.end()) {
            // Matched a keyword, but there mustn't be any alphanumeric
            // or underscore character immediately after. This is to prevent
            // a token like "classd" from being interpreted as "class".
            c = _jack_in.get();
            // Seek 1 character backwards.
            _jack_in.seekg(-1, std::ios_base::cur);

            if (std::isalnum(c) || c == '_') break;

            // Successfully matched a keyword.
            _curr_keyword = get_keyword(token);
            _curr_token_type = TokenType::KEYWORD;
            _curr_token = token;
            return true;
        } else {
            c = _jack_in.get();
            token.push_back(c);
            ++num_read;
        }
    }

    // Seek back to initial position when we fail to read in a valid keyword.
    // It's important to do this in order to re-attempt reading this token as
    // an identifier.
    _jack_in.seekg(-num_read, std::ios_base::cur);

    return false;
}

void LexicalAnalyser::try_read_identifier() {
    std::string token = "";

    // Scan characters into `token` until a non-alphanumeric and non-underscore
    // character is encountered. This will form the identifier.
    char c = _jack_in.get();
    while (std::isalnum(c) || c == '_') {
        token.push_back(c);
        if (_jack_in.eof())
            throw JackSyntaxError("Unexpected EOF while reading identifier.");
        c = _jack_in.get();
    }

    // Seek 1 character backwards.
    _jack_in.seekg(-1, std::ios_base::cur);

    // Validate the identifier.
    if (!std::regex_match(token, valid_identifier_pattern)) {
        std::string err_msg = "Invalid identifier name: '" + token + "'";
        throw JackSyntaxError(err_msg.c_str());
    } else {
        _curr_token = token;
        _curr_token_type = TokenType::IDENTIFIER;
    }
}

// TODO: is there a better way of doing this...?
Keyword LexicalAnalyser::get_keyword(const std::string& keyword) {
    if (keyword == "class")            return Keyword::CLASS;
    else if (keyword == "method")      return Keyword::METHOD;
    else if (keyword == "function")    return Keyword::FUNCTION;
    else if (keyword == "constructor") return Keyword::CONSTRUCTOR;
    else if (keyword == "int")         return Keyword::INT;
    else if (keyword == "boolean")     return Keyword::BOOLEAN;
    else if (keyword == "char")        return Keyword::CHAR;
    else if (keyword == "void")        return Keyword::VOID;
    else if (keyword == "var")         return Keyword::VAR;
    else if (keyword == "static")      return Keyword::STATIC;
    else if (keyword == "field")       return Keyword::FIELD;
    else if (keyword == "let")         return Keyword::LET;
    else if (keyword == "do")          return Keyword::DO;
    else if (keyword == "if")          return Keyword::IF;
    else if (keyword == "else")        return Keyword::ELSE;
    else if (keyword == "while")       return Keyword::WHILE;
    else if (keyword == "return")      return Keyword::RETURN;
    else if (keyword == "true")        return Keyword::TRUE;
    else if (keyword == "false")       return Keyword::FALSE;
    else if (keyword == "null")        return Keyword::NULL_KEYWORD;
    else if (keyword == "this")        return Keyword::THIS;

    return Keyword::UNDEFINED;
}

// TODO: is there a better way of doing this...?
std::string LexicalAnalyser::get_token_type() {
    if (_curr_token_type == TokenType::KEYWORD) return "keyword";
    else if (_curr_token_type == TokenType::SYMBOL) return "symbol";
    else if (_curr_token_type == TokenType::IDENTIFIER) return "identifier";
    else if (_curr_token_type == TokenType::INT_CONST) return "integerConstant";
    else if (_curr_token_type == TokenType::STRING_CONST) return "stringContant";
    else if (_curr_token_type == TokenType::COMMENT) return "comment";

    return "undefined";
}

JackSyntaxError::JackSyntaxError(char const* const message) throw()
    : _message(message) {
}

char const* JackSyntaxError::what() const throw() {
    return _message;
}

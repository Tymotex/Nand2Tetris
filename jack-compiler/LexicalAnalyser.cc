#include "LexicalAnalyser.h"
#include "utils/Colouriser.h"
#include "utils/XMLOutput.h"
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

std::regex LexicalAnalyser::valid_identifier_pattern = std::regex(R"(^[a-zA-Z]\w*$)");

LexicalAnalyser::LexicalAnalyser(const std::string& source_jack_file_path)
    : _jack_in(std::ifstream(source_jack_file_path)),
      _curr_token(""),
      _curr_token_type(TokenType::UNDEFINED),
      _curr_keyword(Keyword::UNDEFINED) {
}

LexicalAnalyser::~LexicalAnalyser() {
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
    } else if (std::isdigit(first_char)) {
        // Take one step back to read in the full token.
        _jack_in.seekg(-1, std::ios_base::cur);
        try_read_int_literal();
    } else if (first_char == '/') {
        char second_char = _jack_in.get();
        if (second_char == '/') {
            try_advance_past_comment(false);
        } else if (second_char == '*') {
            try_advance_past_comment(true);
        } else {
            // It's necessary to disambiguate between comments and the division
            // operator, /.
            _curr_token = std::string{first_char};
            _curr_token_type = TokenType::SYMBOL;
        }
    } else if (symbol_lexicon.find(first_char) != symbol_lexicon.end()) {
        _curr_token = std::string{first_char};
        _curr_token_type = TokenType::SYMBOL;
    } else {
        // Take one step back to read in the full token.
        _jack_in.seekg(-1, std::ios_base::cur);
        if (!try_read_keyword()) {
            try_read_identifier();
        }
    }
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

void LexicalAnalyser::step_back() {
    _jack_in.seekg(-1, std::ios_base::cur);
}

void LexicalAnalyser::reset() {
    _jack_in.clear();
    _jack_in.seekg(0);
    _curr_token = "";
    _curr_token_type = TokenType::UNDEFINED;
    _curr_keyword = Keyword::UNDEFINED;
}

void LexicalAnalyser::write_xml_tokens(const std::string& token_xml_output_path,
        const bool& enable_debug) {
    std::string _token_xml_outpu_path;
    XMLOutput token_xml_out(token_xml_output_path, false, false);

    // Start the XML token stream.
    token_xml_out.open_xml("tokens");

    while (try_advance()) {
        if (enable_debug) show_tokeniser_debug_info();
        // Write the extracted token to the debug info XML output stream.
        if (_curr_token_type != TokenType::COMMENT) {
            token_xml_out.form_xml(get_token_type(), _curr_token);
        }
    }

    token_xml_out.close_xml();
    reset();
}

void LexicalAnalyser::show_tokeniser_debug_info() {
    std::cout << Colour::BLUE;
    switch (token_type()) {
        case TokenType::KEYWORD:
            std::cout << "\tKeyword:    " << get_str_value() << "\n";
            break;
        case TokenType::IDENTIFIER:
            std::cout << "\tIdentifier: " << identifier() << "\n";
            break;
        case TokenType::SYMBOL:
            std::cout << "\tSymbol:     " << symbol() << "\n";
            break;
        case TokenType::INT_CONST:
            std::cout << "\tInt const:  " << get_int_value() << "\n";
            break;
        case TokenType::STRING_CONST:
            std::cout << "\tStr const:  " << get_str_value() << "\n";
            break;
        case TokenType::COMMENT:
            std::cout << "\tComment:    " << get_str_value() << "\n";
            break;
        default:
            break;
    }
    std::cout << Colour::RESET;
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
    _curr_token_type = TokenType::STRING_CONST;
}

// We read int literals based on the following rules:
// - If it's only 1 digit long, then it must be in range 0-9.
//   else if it is multiple digits long, then the leading digit must be non-zero.
//   Eg. 0 and 10 are valid numbers, but 01 and 00 are not.
// - We consider any non-numeric character as a boundary that signals the
//   compiler to stop reading.
void LexicalAnalyser::try_read_int_literal() {
    std::string token = "";

    // Scan digits into token until a non-numeric character is encountered.
    char c;
    while (std::isdigit(c = _jack_in.get())) {
        if (_jack_in.eof()) throw JackSyntaxError("Unexpected EOF while reading integer literal.");
        token.push_back(c);
    }

    // Seek back one character.
    _jack_in.seekg(-1, std::ios_base::cur);

    _curr_token = token;
    _curr_token_type = TokenType::INT_CONST;
}

bool LexicalAnalyser::try_advance_past_comment(const bool& multiline) {
    std::string token = "";
    if (!multiline) {
        // For inline comments, we advance the cursor until the next newline
        // character is encountered. There are no conditions where an inline
        // comment is invalid.
        char c;
        while ((c = _jack_in.get()) != '\n' || _jack_in.eof())
            token.push_back(c);
    } else {
        // For multi-line comments, we advance the cursor until the consecutive
        // characters, '*/', are encountered. If we reach EOF before then, then
        // the comment is invalid.
        bool comment_terminated = false;
        while (!_jack_in.eof()) {
            char c = _jack_in.get();
            if (c == '*') {
                if (_jack_in.eof())
                    throw JackSyntaxError("Unexpected EOF while reading "
                                          "multi-line comment.");
                char c = _jack_in.get();
                if (c == '/') {
                    comment_terminated = true;
                    break;
                }
            }
            token.push_back(c);
        }
        if (!comment_terminated) return false;
    }
    _curr_token_type = TokenType::COMMENT;
    _curr_token = token;
    return true;
}

bool LexicalAnalyser::try_read_keyword() {
    std::string token = "";

    int num_read = 1;
    char c = _jack_in.get();
    token.push_back(c);

    // TODO: this should not be hard-coded... The 11 is meant to be strlen("constructor")
    // TODO: replace this with your own basic implementation of a trie structure.
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
    else if (_curr_token_type == TokenType::STRING_CONST) return "stringConstant";
    else if (_curr_token_type == TokenType::COMMENT) return "comment";

    return "undefined";
}

JackSyntaxError::JackSyntaxError(char const* const message) throw()
    : _message(message) {
}

char const* JackSyntaxError::what() const throw() {
    return _message;
}

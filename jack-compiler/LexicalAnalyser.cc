#include "LexicalAnalyser.h"
#include "utils/Colouriser.h"
#include "utils/XMLOutput.h"
#include <string>
#include <iostream>
#include <unordered_set>
#include <regex>
#include <tuple>

std::unordered_set<std::string> LexicalAnalyser::general_keywords = {
    "class", "var"
};

std::unordered_set<std::string> LexicalAnalyser::statement_keywords = {
    "let", "do", "if", "else", "while", "return"
};

std::unordered_set<std::string> LexicalAnalyser::subroutine_declarers = {
    "constructor", "function", "method"
};

std::unordered_set<std::string> LexicalAnalyser::data_types = {
    "void", "int", "char", "boolean"
};

std::unordered_set<std::string> LexicalAnalyser::field_declarers = {
    "field", "static"
};

std::unordered_set<std::string> LexicalAnalyser::builtin_literals = {
    "true", "false", "null", "this"
};

std::unordered_set<std::string> LexicalAnalyser::general_symbols = {
    "{", "}", "(", ")", "[", "]", ".", ",", ";"
};

std::unordered_set<std::string> LexicalAnalyser::binary_operators = {
    "+", "-", "*", "/", "&", "|", "<", ">", "="
};

std::unordered_set<std::string> LexicalAnalyser::unary_operators = {
    "-", "~"
};

std::unordered_map<std::string, Keyword> LexicalAnalyser::string_to_keyword = {
    {"class", Keyword::CLASS},
    {"method", Keyword::METHOD},
    {"function", Keyword::FUNCTION},
    {"constructor", Keyword::CONSTRUCTOR},
    {"int", Keyword::INT},
    {"boolean", Keyword::BOOLEAN},
    {"char", Keyword::CHAR},
    {"void", Keyword::VOID},
    {"var", Keyword::VAR},
    {"static", Keyword::STATIC},
    {"field", Keyword::FIELD},
    {"let", Keyword::LET},
    {"do", Keyword::DO},
    {"if", Keyword::IF},
    {"else", Keyword::ELSE},
    {"while", Keyword::WHILE},
    {"return", Keyword::RETURN},
    {"true", Keyword::TRUE},
    {"false", Keyword::FALSE},
    {"null", Keyword::NULL_KEYWORD},
    {"this", Keyword::THIS}
};

std::unordered_map<TokenType, std::string> LexicalAnalyser::token_type_to_string = {
    {TokenType::KEYWORD, "keyword"},
    {TokenType::SYMBOL, "symbol"},
    {TokenType::IDENTIFIER, "identifier"},
    {TokenType::INT_CONST, "integerConstant"},
    {TokenType::STRING_CONST, "stringConstant"},
    {TokenType::COMMENT, "comment"}
};

std::regex LexicalAnalyser::valid_identifier_pattern = std::regex(R"(^[a-zA-Z]\w*$)");

LexicalAnalyser::LexicalAnalyser(std::istream& jack_stream, std::ostream& token_xml_stream)
    : _jack_in(jack_stream),
      _token_xml_out(token_xml_stream),
      _curr_token(""),
      _curr_token_type(TokenType::UNDEFINED),
      _curr_keyword(Keyword::UNDEFINED) {
    // Dump the token stream as XML.
    write_xml_tokens(false);
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

    // Reset cursor variables.
    _curr_token = "";
    _curr_keyword = Keyword::UNDEFINED;
    _curr_token_type = TokenType::UNDEFINED;

    // Disambiguate token type based on the first character of the token.
    if (first_char == '"') {
        try_read_string_literal();
    } else if (std::isdigit(first_char)) {
        // Take one step back to read in the full token.
        _jack_in.seekg(-1, std::ios_base::cur);
        try_read_int_literal();
    } else if (first_char == '/') {
        // Peek at the following character.
        char second_char = _jack_in.get();
        _jack_in.seekg(-1, std::ios_base::cur);

        if (second_char == '/') {
            try_advance_past_comment(false);
            // Retry advance.
            try_advance();
        } else if (second_char == '*') {
            try_advance_past_comment(true);
            // Retry advance.
            try_advance();
        } else {
            // It's necessary to disambiguate between comments and the division
            // operator, /.
            _curr_token = std::string{first_char};
            _curr_token_type = TokenType::SYMBOL;
        }
    } else if (is_symbol(std::string{first_char})) {
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
    return get_token();
}

int LexicalAnalyser::get_int_value() {
    return stoi(_curr_token);
}

std::string LexicalAnalyser::get_token() {
    return _curr_token;
}

std::string LexicalAnalyser::peek() {
    try_advance();
    std::string token = _curr_token;
    step_back();
    return token;
}

void LexicalAnalyser::step_back() {
    // Note: for string constants, we need to step back 2 extra bytes because
    //       to account for the 2 double quotes "".
    if (_curr_token_type == TokenType::STRING_CONST) 
        _jack_in.seekg(-(_curr_token.size() + 2), std::ios_base::cur);
    else 
        _jack_in.seekg(-_curr_token.size(), std::ios_base::cur);
}

void LexicalAnalyser::reset() {
    _jack_in.clear();
    _jack_in.seekg(0);
    _curr_token = "";
    _curr_token_type = TokenType::UNDEFINED;
    _curr_keyword = Keyword::UNDEFINED;
}

void LexicalAnalyser::write_xml_tokens(const bool& enable_debug) {
    XMLOutput xml_writer(_token_xml_out, false, false);

    // Start the XML token stream.
    xml_writer.open_xml("tokens");

    while (try_advance()) {
        if (enable_debug) show_tokeniser_debug_info();
        // Write the extracted token to the debug info XML output stream.
        if (_curr_token_type != TokenType::COMMENT) {
            xml_writer.form_xml(get_token_type(), _curr_token);
        }
    }

    xml_writer.close_xml();
    reset();
}

bool LexicalAnalyser::try_advance_until_class_declaration() {
    while (true) {
        if (token_type() == TokenType::KEYWORD && keyword() == Keyword::CLASS)
            return true;
        if (!try_advance())
            return false;
    }
    return false;
}

void LexicalAnalyser::show_tokeniser_debug_info() {
    std::cout << Colour::BLUE;
    switch (token_type()) {
        case TokenType::KEYWORD:
            std::cout << "\tKeyword:    " << get_token() << "\n";
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
            std::cout << "\tStr const:  " << get_token() << "\n";
            break;
        case TokenType::COMMENT:
            std::cout << "\tComment:    " << get_token() << "\n";
            break;
        default:
            break;
    }
    std::cout << Colour::RESET;
}

void LexicalAnalyser::try_advance_past_whitespace() {
    while (!(_jack_in.eof())) {
        char c = _jack_in.get();
        if (!std::isspace(c)) {
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
            throw JackSyntaxError(*this, "Expected \" before newline or EOF while reading string literal.");
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
        if (_jack_in.eof()) throw JackSyntaxError(*this, "Unexpected EOF while reading integer literal.");
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
                    throw JackSyntaxError(*this, "Unexpected EOF while reading "
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

bool LexicalAnalyser::is_prefix_of_any_keyword(const std::string& token, std::unordered_set<std::string>& lexicon) {
    for (const std::string& keyword : lexicon)
        if (std::mismatch(token.begin(), token.end(), keyword.begin()).first == token.end()) 
            return true;
    return false;
}

bool LexicalAnalyser::try_read_keyword() {
    std::string token = "";

    int num_read = 1;
    char c = _jack_in.get();
    token.push_back(c);

    while (is_prefix_of_any_keyword(token, general_keywords) ||
           is_prefix_of_any_keyword(token, statement_keywords) ||
           is_prefix_of_any_keyword(token, subroutine_declarers) ||
           is_prefix_of_any_keyword(token, data_types) ||
           is_prefix_of_any_keyword(token, field_declarers) ||
           is_prefix_of_any_keyword(token, builtin_literals)) {
        if (_jack_in.eof())
            throw JackSyntaxError(*this, "Unexpected EOF while reading Jack keyword.");
        if (is_keyword(token)) {
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
            throw JackSyntaxError(*this, "Unexpected EOF while reading identifier.");
        c = _jack_in.get();
    }

    // Seek 1 character backwards.
    if (!_jack_in.eof())
        _jack_in.seekg(-1, std::ios_base::cur);

    // Validate the identifier.
    if (!std::regex_match(token, valid_identifier_pattern)) {
        std::string err_msg = "Invalid identifier name: '" + token + "'";
        throw JackSyntaxError(*this, err_msg.c_str());
    } else {
        _curr_token = token;
        _curr_token_type = TokenType::IDENTIFIER;
    }
}

bool LexicalAnalyser::is_keyword(const std::string& token) {
    return (general_keywords.find(token) != general_keywords.end()) ||
        (statement_keywords.find(token) != statement_keywords.end()) || 
        (subroutine_declarers.find(token) != subroutine_declarers.end()) || 
        (data_types.find(token) != data_types.end()) || 
        (field_declarers.find(token) != field_declarers.end()) || 
        (builtin_literals.find(token) != builtin_literals.end());
}

bool LexicalAnalyser::is_symbol(const std::string& token) {
    return (general_symbols.find(token) != general_symbols.end()) || 
        (binary_operators.find(token) != binary_operators.end()) || 
        (unary_operators.find(token) != unary_operators.end());
}

Keyword LexicalAnalyser::get_keyword(const std::string& keyword) {
    if (string_to_keyword.find(keyword) != string_to_keyword.end()) {
        return string_to_keyword[keyword];
    }
    return Keyword::UNDEFINED;
}

std::string LexicalAnalyser::get_token_type() {
    if (token_type_to_string.find(token_type()) != token_type_to_string.end()) {
        return token_type_to_string[token_type()];
    }
    return "undefined";
}

JackSyntaxError::JackSyntaxError(LexicalAnalyser& lexical_analyser, char const* const message) throw()
        : _message(message) {
    std::cerr << Colour::RED
              << "Syntax Error: " 
              << message
              << Colour::RESET
              << std::endl;
}

char const* JackSyntaxError::what() const throw() {
    return _message;
}

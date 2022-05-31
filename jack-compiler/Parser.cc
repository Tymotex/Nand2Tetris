#include "LexicalAnalyser.h"
#include "Parser.h"
#include "utils/XMLOutput.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <unordered_set>

// TODO: Refactor keyword_lexicon
// TODO: should make try_advance throw error on *unexpected* EOF.
// TODO: attach line numbers to all parse errors (and syntax errors). The error classes should be passed this extra info.

Parser::Parser(std::shared_ptr<LexicalAnalyser> lexical_analyser, const std::string& output_stream)
    : _lexical_analyser(lexical_analyser),
      _xml_parse_tree(std::make_unique<XMLOutput>(output_stream, true)) {
}

void Parser::compile_class() {
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected class name.");
    if (expect_token("{"))
        throw JackParserError("Expected class scope starter, {.");
    compile_class_body();
}

// TODO: probably need to refactor keyword_lexicon.
void Parser::compile_class_body() {
    // We expect an arbitrary stream of either subroutine declarations or field
    // declarations.
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        if (curr_token == "constructor" || curr_token == "function" || curr_token == "method") {
            compile_subroutine();
        } else if (curr_token == "field" || curr_token == "static") {
            compile_class_field_declaration();
        } else {
            throw JackParserError("Unexpected token in class body. Expected "
                                  "subroutine or field declaration");
        }
    }
}

void Parser::compile_class_field_declaration() {
    // TODO: support 'static'|'field' type varName, varName, varName, ...;
    if (!expect_data_type())
        throw JackParserError("Expected a data type for field declaration.");
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected an identifier for field declaration.");
    if (!expect_token(";"))
        throw JackParserError("Unterminated field declaration.");
}

void Parser::compile_subroutine() {
    if (!expect_data_type())
        throw JackParserError("Expected subroutine return type.");
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected subroutine name.");
    compile_parameter_list();
    compile_subroutine_body();
}

void Parser::compile_parameter_list() {
    expect_token("(");
    std::string curr_token = "";
    int param_num = 1;
    while (_lexical_analyser->try_advance() && (_lexical_analyser->get_str_value() != ")")) {
        // Take one step back since we looked ahead to check for ).
        _lexical_analyser->step_back();

        // Parameter lists must always be of form: `type identifier` .
        if (!expect_data_type()) {
            // TODO: this is such a dumb way of forming a c-str. Alternatives?
            std::stringstream err_msg;
            err_msg << "Expected data type for parameter " << param_num;
            throw JackParserError(err_msg.str().c_str());
        }
        if (!expect_token_type(TokenType::IDENTIFIER)) {
            // TODO: this is such a dumb way of forming a c-str. Alternatives?
            std::stringstream err_msg;
            err_msg << "Expected identifier for parameter " << param_num;
            throw JackParserError(err_msg.str().c_str());
        }
        ++param_num;
    }
}

void Parser::compile_subroutine_body() {
    expect_token("{");
    compile_statements();
}

// Note: `compile_statements` will terminate when the closing brace character is
//       encountered.
void Parser::compile_statements() {
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        if (curr_token == "}") break;
        
        // TODO: remove braces {}
        if (curr_token == "let") {
            compile_let();
        } else if (curr_token == "if") {
            compile_if();
        } else if (curr_token == "var") {
            compile_variable_declaration();
        } else if (curr_token == "do") {
            compile_do();
        } else if (curr_token == "while") {
            compile_while();
        } else if (curr_token == "return") {
            compile_return();
        }
    }
}

void Parser::compile_variable_declaration() {
    // TODO: support form: `var type varName, anotherVar, otherVar, ...;`
    if (!expect_data_type())
        throw JackParserError("Expected data type for variable declaration.");
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected identifier for variable declaration.");
    if (!expect_token(";"))
        throw JackParserError("Unterminated variable declaration statement");
}

void Parser::compile_let() {
    // TODO: support let varName[expr] = expression;
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected variable identifier for let statement");
    if (!expect_token("="))
        throw JackParserError("Expected assignment operator for let statement");
    compile_expression();
    if (!expect_token(";"))
        throw JackParserError("Unterminated let statement.");
}

void Parser::compile_if() {
    // TODO: support if (expr) { statements } else { statements }
    if (!expect_token("("))
        throw JackParserError("Expected start of condition for if-statement.");
    compile_expression();
    if (!expect_token(")"))
        throw JackParserError("Expected end of condition for if-statement.");
    if (!expect_token("{"))
        throw JackParserError("Expected start of if-statement scope.");
    compile_statements();
}

void Parser::compile_while() {
    if (!expect_token("("))
        throw JackParserError("Expected start of condition for while-loop.");
    compile_expression();
    if (!expect_token(")"))
        throw JackParserError("Expected end of condition for while-loop.");
    if (!expect_token("{"))
        throw JackParserError("Expected start of if-statement scope.");
    compile_statements();
}

void Parser::compile_do() {
    // In Jack, a subroutine call is only ever present in `do` statements and
    // in expressions. Here, we are reusing the `compile_expression` algorithm
    // to compile direct subroutine calls of the form `do subroutine_call()`.
    compile_expression();
    if (!expect_token(";"))
        throw JackParserError("Unterminated do statement.");
}

void Parser::compile_return() {
    compile_expression();
    if (!expect_token(";"))
        throw JackParserError("Unterminated return statement.");
}

// When compiling expressions, we expect the following terms:
// - int literal
// - string literal
// - true/false
// - null
// - this
// - identifier (variable name)
// - subscripted identifier (variable array access)
// - nested-expressions with ()
// - subroutine calls
// 
// Any of the above can occur with a unary operator such as ~ or -. 
// Otherwise we expect a binary operator to exist between multi-term
// expressions, such as +, *, &, |, etc. Ie. we use these binary operators
// to help delimit terms in the expression. It is invalid to have 2 terms
// consecutively without a binary operator between them.
//
// Eg. `let x = y+arr[5]-p.get(row)*count()-Math.sqrt(dist)/2;`
// 
//     The expression on the RHS consists of 6 terms:
//         y, arr[5], p.get(row), count(), Math.sqrt(dist), 2
// 
//     Expressed in an XML parse tree, we'd have
//         <expression>
//             <term>
//                 <identifier> y </identifier>
//             </term>
//             <symbol> + </symbol>
//             <term>
//                 <identifier> arr </identifier>
//                 <symbol> [ </symbol>
//                 <expression>
//                     <integerConstant> 5 </integerConstant>
//                 </expression>
//                 <symbol> ] </symbol>
//             </term>
//             ... and so on
//         </expression>
// 
// Eg. `if (((y + size) < 254) & ((x + size) < 510)) { ... }`
//     The conditional expression consists of 2 terms, but both of them 
//     denote sub-expressions that should be recursively decomposed.
//         ((y + size) < 254)
//             (y + size)
//                 y
//                 size
//             254
//         ((x + size) < 510)
//             (x + size)
//                 x
//                 size
//             510
// 
//     Expressed in an XML parse tree, we'd have
//         <expression>
//             <term>
//               <symbol> ( </symbol>
//               <expression>
//                 <term>
//                   <symbol> ( </symbol>
//                   <expression>
//                     <term>
//                       <identifier> y </identifier>
//                     </term>
//                     <symbol> + </symbol>
//                     <term>
//                       <identifier> size </identifier>
//                     </term>
//                   </expression>
//                   <symbol> ) </symbol>
//                 </term>
//                 <symbol> &lt; </symbol>
//                 <term>
//                   <integerConstant> 254 </integerConstant>
//                 </term>
//               </expression>
//               <symbol> ) </symbol>
//             </term>
//             <symbol> &amp; </symbol>
//             <term>
//               <symbol> ( </symbol>
//               <expression>
//                 <term>
//                   <symbol> ( </symbol>
//                   <expression>
//                     <term>
//                       <identifier> x </identifier>
//                     </term>
//                     <symbol> + </symbol>
//                     <term>
//                       <identifier> size </identifier>
//                     </term>
//                   </expression>
//                   <symbol> ) </symbol>
//                 </term>
//                 <symbol> &lt; </symbol>
//                 <term>
//                   <integerConstant> 510 </integerConstant>
//                 </term>
//               </expression>
//               <symbol> ) </symbol>
//             </term>
//         </expression>
void Parser::compile_expression() {
    // Process the first term, then handle subsequent operators and terms.
    // There must be at least a single term for the expression to be valid.
    compile_term();

    std::string curr_token;
    std::unordered_set<std::string> binary_operators = {
        "+", "-", "*", "/", "&", "|", "<", ">", "="
    };

    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        // We expect the next token to be a binary operator, otherwise we stop
        // compiling the expression.
        if (binary_operators.find(curr_token) != binary_operators.end()) {
            compile_term();
        } else {
            // Take a step back since we looked ahead.
            _lexical_analyser->step_back();
            break;
        }
    }
}

void Parser::compile_term() {
    _lexical_analyser->try_advance();
    std::string curr_token = _lexical_analyser->get_str_value();
    std::string peeked_token;
    TokenType token_type = _lexical_analyser->token_type();

    // TODO: these should be moved.
    std::unordered_set<std::string> unary_operators = {
        "-", "~"
    };
    std::unordered_set<std::string> builtin_literals = {
        "true", "false", "null", "this"
    };

    switch (token_type) {
        case TokenType::INT_CONST:
            break;
        case TokenType::STRING_CONST:
            break;
        case TokenType::KEYWORD:
            if (builtin_literals.find(curr_token) != builtin_literals.end()) {

            } else {
                // TODO: this is dumb. fix
                std::stringstream err_msg;
                err_msg << "Invalid keyword for term '" << curr_token << "'.";
                throw JackParserError(err_msg.str().c_str());
            }
            break;
        case TokenType::IDENTIFIER:
            // TODO: support array access: `varName[expr]`
            // We need to look ahead one character to ascertain whether this
            // term is a subroutine call or a reference to a variable.
            _lexical_analyser->try_advance();
            peeked_token = _lexical_analyser->get_str_value();
            _lexical_analyser->step_back();
            // TODO: this peek/look-ahead operation is common. Maybe make a method out of it in LexicalAnalyser.

            if (peeked_token == "(" || peeked_token == ".") {
                // Compile subroutine call.
                compile_subroutine_invocation();
            } else {
                // Compile a regular variable identifier.
                
            }
            break;
        case TokenType::SYMBOL:
            if (curr_token == "(") {
                // Compile a sub-expression.
                compile_expression();
            } else if (unary_operators.find(curr_token) != unary_operators.end()) {

            } else {
                // TODO: this is dumb. fix
                std::stringstream err_msg;
                err_msg << "Unexpected expression symbol '" << curr_token << "'.";
                throw JackParserError(err_msg.str().c_str());
            }
            break;
    }
}

void Parser::compile_subroutine_invocation() {
    _lexical_analyser->try_advance();
    std::string token = _lexical_analyser->get_str_value();

    if (token == "(") {
        compile_expression_list();
    } else if (token == ".") {
        // Follow the '.' chain down to the subroutine invocation.
        // Eg. if the token stream consisted of `MyClass.myMethod(...)`, then
        //     the recursive call would process `myMethod(...)`.
        expect_token_type(TokenType::IDENTIFIER);
        compile_subroutine_invocation();
        // TODO: I don't have much faith this works.
    } else {
        throw JackParserError("Invalid subroutine invocation.");
    }
}

int Parser::compile_expression_list() {
    // There must exist at least 1 expression for the expression list to be
    // valid.
    int num_expressions = 1;
    compile_expression();

    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();

        // End expression list compilation when ) is reached.
        if (curr_token == ")") break;

        // We expect the next token to be a comma followed by another
        // expression.
        if (curr_token == ",") {
            compile_expression();
        } else {
            throw JackParserError("Expected comma between expressions.");
        }

        ++num_expressions;
    }

    return num_expressions;
}

bool Parser::expect_token_type(TokenType token_type) {
    _lexical_analyser->try_advance();
    return _lexical_analyser->token_type() == token_type;
}

bool Parser::expect_token(const std::string& token) {
    _lexical_analyser->try_advance();
    std::string curr_token = _lexical_analyser->get_str_value();
    return curr_token == token;
}

// A valid data type is either a built-in type or an identifier (it's not the
// responsibility of this function to determine whether it actually references
// a valid class (TODO: yet?)).
bool Parser::expect_data_type() {
    std::unordered_set<std::string> data_types = {
        "void", "int", "char", "boolean"
    };
    _lexical_analyser->try_advance();
    std::string curr_token = _lexical_analyser->get_str_value();
    return (data_types.find(curr_token) != data_types.end()) ||
        _lexical_analyser->token_type() == TokenType::IDENTIFIER;
}

JackParserError::JackParserError(char const* const message) throw()
    : _message(message) {
}

char const* JackParserError::what() const throw() {
    return _message;
}

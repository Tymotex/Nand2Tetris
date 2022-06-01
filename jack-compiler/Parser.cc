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
      _xml_parse_tree(std::make_unique<XMLOutput>(output_stream, true, true)) {
}

Parser::~Parser() {
    _xml_parse_tree->close();
}

void Parser::compile_class() {
    _xml_parse_tree->open_xml("class");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value()); // TODO: this line gets duplicated a lot.
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected class name.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("{"))
        throw JackParserError("Expected class scope starter, {.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_class_body();
    _xml_parse_tree->close_xml();  // TODO: this line is also duplicated many times.
}

// TODO: probably need to refactor keyword_lexicon.
void Parser::compile_class_body() {
    // We expect an arbitrary stream of either subroutine declarations or field
    // declarations.
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        if (curr_token == "}") {
            // Reached the end of the class implementation.
            _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
            break;
        }

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
    _xml_parse_tree->open_xml("subroutineDec");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_data_type())
        throw JackParserError("Expected subroutine return type.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected subroutine name.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());

    if (!expect_token("("))
        throw JackParserError("Expected start of parameter list.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value()); // TODO: document that this inserts (
    compile_parameter_list();

    compile_subroutine_body();
    _xml_parse_tree->close_xml();
}

void Parser::compile_parameter_list() {
    _xml_parse_tree->open_xml("parameterList");
    std::string curr_token = "";
    int param_num = 1;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        if (curr_token == ")") {
            _xml_parse_tree->close_xml();
            _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
            break;
        }

        // Take one step back since we looked ahead to check for ).
        _lexical_analyser->step_back();

        // For more than one parameter, we expect a comma.
        if (param_num > 1) {
            if (!expect_token(","))
                throw JackParserError("Expected comma in parameter list.");
            else _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
        } 

        // Parameter lists must always be of form: `type identifier` .
        if (!expect_data_type()) {
            // TODO: this is such a dumb way of forming a c-str. Alternatives?
            std::stringstream err_msg;
            err_msg << "Expected data type for parameter " << param_num;
            throw JackParserError(err_msg.str().c_str());
        }
        _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
        if (!expect_token_type(TokenType::IDENTIFIER)) {
            // TODO: this is such a dumb way of forming a c-str. Alternatives?
            std::stringstream err_msg;
            err_msg << "Expected identifier for parameter " << param_num;
            throw JackParserError(err_msg.str().c_str());
        }
        _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
        ++param_num;
    }
}

void Parser::compile_subroutine_body() {
    _xml_parse_tree->open_xml("subroutineBody");
    expect_token("{");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_statements();
    _xml_parse_tree->close_xml();
}

// Note: `compile_statements` will terminate when the closing brace character is
//       encountered.
void Parser::compile_statements() {
    _xml_parse_tree->open_xml("statements");
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();
        if (curr_token == "}") {
            _xml_parse_tree->close_xml();
            _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value()); // TODO: this is the final }
            break;
        }
        
        if (curr_token == "let")         compile_let();
        else if (curr_token == "if")     compile_if();
        else if (curr_token == "var")    compile_variable_declaration();
        else if (curr_token == "do")     compile_do();
        else if (curr_token == "while")  compile_while();
        else if (curr_token == "return") compile_return();
        else throw JackParserError("Start of unexpected statement.");
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
    _xml_parse_tree->open_xml("letStatement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected variable identifier for let statement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("="))
        throw JackParserError("Expected assignment operator for let statement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_expression();
    if (!expect_token(";"))
        throw JackParserError("Unterminated let statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    _xml_parse_tree->close_xml();
}

void Parser::compile_if() {
    // TODO: support if (expr) { statements } else { statements }
    _xml_parse_tree->open_xml("ifStatement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("("))
        throw JackParserError("Expected start of condition for if-statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_expression();
    if (!expect_token(")"))
        throw JackParserError("Expected end of condition for if-statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("{"))
        throw JackParserError("Expected start of if-statement scope.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_statements();
    _xml_parse_tree->close_xml();
}

void Parser::compile_while() {
    _xml_parse_tree->open_xml("whileStatement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("("))
        throw JackParserError("Expected start of condition for while-loop.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_expression();
    if (!expect_token(")"))
        throw JackParserError("Expected end of condition for while-loop.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    if (!expect_token("{"))
        throw JackParserError("Expected start of if-statement scope.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    compile_statements();
    _xml_parse_tree->close_xml();
}

void Parser::compile_do() {
    // In Jack, a subroutine call is only ever present in `do` statements and
    // in expressions. Here, we are reusing the `compile_expression` algorithm
    // to compile direct subroutine calls of the form `do subroutine_call()`.
    _xml_parse_tree->open_xml("doStatement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    
    if (!expect_token_type(TokenType::IDENTIFIER))
        throw JackParserError("Expected identifier for subroutine invocation in do statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value()); // subroutineName.
    compile_subroutine_invocation();

    if (!expect_token(";"))
        throw JackParserError("Unterminated do statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    _xml_parse_tree->close_xml();
}

void Parser::compile_return() {
    _xml_parse_tree->open_xml("returnStatement");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());

    if (_lexical_analyser->try_advance() && _lexical_analyser->get_str_value() != ";") {
        // There is a return value. We now resolve the expression.
        compile_expression();
    } else {
        _lexical_analyser->step_back();
    }

    if (!expect_token(";"))
        throw JackParserError("Unterminated return statement.");
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
    _xml_parse_tree->close_xml();
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
    _xml_parse_tree->open_xml("expression");

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
            _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
            compile_term();
        } else {
            // Take a step back since we looked ahead.
            _lexical_analyser->step_back();
            break;
        }
    }
    _xml_parse_tree->close_xml();
}

void Parser::compile_term() {
    _xml_parse_tree->open_xml("term");
    _lexical_analyser->try_advance();
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());

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
        // TODO: remove?
        // case TokenType::INT_CONST:
        //     break;
        // case TokenType::STRING_CONST:
        //     break;
        case TokenType::KEYWORD:
            if (builtin_literals.find(curr_token) == builtin_literals.end()) {
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
            }
            break;
        case TokenType::SYMBOL:
            if (curr_token == "(") {
                // Compile a sub-expression.
                compile_expression();
            } else if (unary_operators.find(curr_token) != unary_operators.end()) {
                // When a unary operator is encountered, we expect a term to 
                // immediately follow.
                compile_term();
            } else {
                // TODO: this is dumb. fix
                std::stringstream err_msg;
                err_msg << "Unexpected expression symbol '" << curr_token << "'.";
                throw JackParserError(err_msg.str().c_str());
            }
            break;
        default:
            break;
    }
    _xml_parse_tree->close_xml();
}

void Parser::compile_subroutine_invocation() {
    _lexical_analyser->try_advance();
    std::string token = _lexical_analyser->get_str_value();
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value()); // ( or .

    if (token == "(") {
        compile_expression_list();
    } else if (token == ".") {
        // Follow the '.' chain down to the subroutine invocation.
        // Eg. if the token stream consisted of `MyClass.myMethod(...)`, then
        //     the recursive call would process `myMethod(...)`.
        expect_token_type(TokenType::IDENTIFIER);
        _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
        compile_subroutine_invocation();
    } else {
        throw JackParserError("Invalid subroutine invocation.");
    }
}

int Parser::compile_expression_list() {
    // There must exist at least 1 expression for the expression list to be
    // valid.
    _xml_parse_tree->open_xml("expressionList");
    int num_expressions = 1;
    compile_expression();

    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_str_value();

        // End expression list compilation when ) is reached.
        if (curr_token == ")") {
            _xml_parse_tree->close_xml();
            _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), _lexical_analyser->get_str_value());
            break;
        }

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

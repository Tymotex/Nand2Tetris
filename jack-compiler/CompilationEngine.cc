#include "LexicalAnalyser.h"
#include "CompilationEngine.h"
#include "utils/XMLOutput.h"
#include "utils/Colouriser.h"
#include <iostream>
#include <fstream>
#include <memory>

CompilationEngine::CompilationEngine(
        std::shared_ptr<LexicalAnalyser> lexical_analyser,
        std::ostream& vm_stream, std::ostream& xml_stream)
        : _lexical_analyser(lexical_analyser),
          _vm_out(vm_stream),
          _xml_parse_tree(std::make_unique<XMLOutput>(xml_stream, true, false)) {
}

CompilationEngine::~CompilationEngine() {
    _xml_parse_tree->close();
}

// Class declarations are of the form:
//     class className { body }
void CompilationEngine::compile_class() {
    _xml_parse_tree->open_xml("class");

    // class
    xml_capture_token();

    // className
    expect_token_type(TokenType::IDENTIFIER, "Expected class name.");

    // { body }
    expect_token("{", "Expected class scope starter, {.");
    compile_class_body();
    expect_token("}", "Expected class scope terminator, {.");

    _xml_parse_tree->close_xml();
}

// Class bodies are of the form:
//     {  classVarDec* subroutineDec* }
void CompilationEngine::compile_class_body() {
    // We expect an arbitrary stream of either subroutine declarations or field
    // declarations.
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_token();
        if (curr_token == "}") {
            // Reached the end of the class implementation.
            _lexical_analyser->step_back();
            break;
        }

        if (curr_token == "field" || curr_token == "static") {
            compile_class_field_declaration();
        } else if (curr_token == "constructor" || curr_token == "function" || curr_token == "method") {
            compile_subroutine();
        } else {
            throw JackCompilationEngineError(*_lexical_analyser, 
                "Unexpected token in class body. Expected subroutine or field declaration.");
        }
    }
}

// Class field declarations can be of form:
//     static|field type varName;
//     static|field type varName1, varName2, ...;
void CompilationEngine::compile_class_field_declaration() {
    _xml_parse_tree->open_xml("classVarDec");

    // static|field
    std::string decl_type = xml_capture_token();

    // type
    std::string data_type = expect_data_type("Expected a data type for field declaration.");

    // varName
    std::string identifier = expect_token_type(TokenType::IDENTIFIER, "Expected an identifier for field declaration.");

    // Optional trailing variable list: ', varName2, varName3, ...'
    try_compile_trailing_variable_list();
    
    // ;
    expect_token(";", "Unterminated field declaration.");

    // Record the identifier in the symbol table.
    _class_symbol_table.define(identifier, data_type, decl_type);
    
    _xml_parse_tree->close_xml();
}

// Subroutine declarations are of the form:
//     constructor|function|method type subroutineName (parameterList) { body }
void CompilationEngine::compile_subroutine() {
    _xml_parse_tree->open_xml("subroutineDec");

    // constructor|function|method
    xml_capture_token();

    // type
    expect_data_type("Expected subroutine return type.");

    // subroutineName
    expect_token_type(TokenType::IDENTIFIER, "Expected subroutine name.");

    // (parameterList)
    expect_token("(", "Expected start of parameter list.");
    compile_parameter_list();
    expect_token(")", "Expected end of parameter list.");

    // { body }
    compile_subroutine_body();
    _xml_parse_tree->close_xml();
}

// Parameter lists are of the form:
//     (type identifier)
//     (type identifier1, type identifier2, ...)
void CompilationEngine::compile_parameter_list() {
    _xml_parse_tree->open_xml("parameterList");
    std::string curr_token = "";
    int param_num = 1;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_token();
        if (curr_token == ")") {
            _lexical_analyser->step_back();
            _xml_parse_tree->close_xml();
            break;
        }

        // Take one step back since we looked ahead to check for ).
        _lexical_analyser->step_back();

        // For more than one parameter, we expect a comma.
        if (param_num > 1)
            expect_token(",", "Expected comma in parameter list.");

        // Parameter lists must always be of form: `type identifier` .
        expect_data_type("Expected data type for parameter " + param_num);

        expect_token_type(TokenType::IDENTIFIER, "Expected identifier for parameter " + param_num);

        ++param_num;
    }
}

// Subroutine bodies are of the form:
//     { varDeclarations* statement* }
void CompilationEngine::compile_subroutine_body() {
    _xml_parse_tree->open_xml("subroutineBody");

    // { varDeclarations*
    expect_token("{", "Expected start of subroutine scope.");

    while (_lexical_analyser->try_advance() && _lexical_analyser->get_token() == "var") {
        compile_variable_declaration();
    }
    _lexical_analyser->step_back();

    // statement* }
    compile_statements();
    expect_token("}", "Expected end of subroutine scope.");

    _xml_parse_tree->close_xml();
}

// Note: `compile_statements` will terminate when the closing brace character is
//       encountered.
void CompilationEngine::compile_statements() {
    _xml_parse_tree->open_xml("statements");
    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_token();
        if (curr_token == "}") {
            _lexical_analyser->step_back();
            _xml_parse_tree->close_xml();
            break;
        }
        
        if (curr_token == "let")         compile_let();
        else if (curr_token == "if")     compile_if();
        else if (curr_token == "do")     compile_do();
        else if (curr_token == "while")  compile_while();
        else if (curr_token == "return") compile_return();
        else if (curr_token == "var")
            throw JackCompilationEngineError(*_lexical_analyser, "Variable declarations must precede statements.");
        else
            throw JackCompilationEngineError(*_lexical_analyser, "Start of unexpected statement.");
    }
}

// Variable declarations appearing in subroutine contexts can be of form:
//     var type varName;
//     var type varName1, varName2, ...;
void CompilationEngine::compile_variable_declaration() {
    _xml_parse_tree->open_xml("varDec");
    
    // var
    xml_capture_token();

    // type
    expect_data_type("Expected data type for variable declaration.");

    // varName
    expect_token_type(TokenType::IDENTIFIER, "Expected identifier for variable declaration.");

    // Optional trailing variable list: ', varName2, varName3, ...'
    try_compile_trailing_variable_list();

    // ;
    expect_token(";", "Unterminated variable declaration statement");
    _xml_parse_tree->close_xml();
}

// Let statements can be of form:
//     let varName = expression;
//     let varName[expression] = expression;
void CompilationEngine::compile_let() {
    _xml_parse_tree->open_xml("letStatement");
    
    // let
    xml_capture_token();
    
    // varName
    expect_token_type(TokenType::IDENTIFIER, "Expected variable identifier for let statement");

    // Optional: subscript operator, '[expression]'
    try_compile_subscript();
    
    // =
    expect_token("=", "Expected assignment operator for let statement");

    // expression
    compile_expression(0);
    
    // ;
    expect_token(";", "Unterminated let statement.");

    _xml_parse_tree->close_xml();
}

// If-statements can be of the form:
//     if (expression) { statements }
//     if (expression) { statements } else { statements }
void CompilationEngine::compile_if() {
    _xml_parse_tree->open_xml("ifStatement");
    
    // if
    xml_capture_token();
    
    // (expression)
    expect_token("(", "Expected start of condition for if-statement.");
    compile_expression(0);
    expect_token(")", "Expected end of condition for if-statement.");

    // { statements }
    expect_token("{", "Expected start of if-statement scope.");
    compile_statements();
    expect_token("}", "Expected end of if-statement scope.");

    // Optional: else { statements }
    _lexical_analyser->try_advance();
    if (_lexical_analyser->get_token() == "else") {
        xml_capture_token();
        expect_token("{", "Expected start of else-statement scope.");
        compile_statements();
        expect_token("}", "Expected end of if-statement scope.");
    } else {
        _lexical_analyser->step_back();
    }

    _xml_parse_tree->close_xml();
}

// While statements can be of form:
//     while (expression) { statements }
void CompilationEngine::compile_while() {
    _xml_parse_tree->open_xml("whileStatement");
    
    // while
    xml_capture_token();

    // (expression)
    expect_token("(", "Expected start of condition for while-loop.");
    compile_expression(0);
    expect_token(")", "Expected end of condition for while-loop.");

    // { statements }
    expect_token("{", "Expected start of while-loop scope.");
    compile_statements();
    expect_token("}", "Expected end of while-loop scope.");

    _xml_parse_tree->close_xml();
}

// Do statements are of the form:
//     do subroutineInvocation
void CompilationEngine::compile_do() {
    // In Jack, a subroutine call is only ever present in `do` statements and
    // in expressions. Here, we are reusing the `compile_expression` algorithm
    // to compile direct subroutine calls of the form `do subroutine_call()`.
    _xml_parse_tree->open_xml("doStatement");
    xml_capture_token();
    
    expect_token_type(TokenType::IDENTIFIER, "Expected identifier for subroutine invocation in do statement.");
    compile_subroutine_invocation();

    expect_token(";", "Unterminated do statement.");
    _xml_parse_tree->close_xml();
}

// Return statements are of the form:
//     return;
//     return expression;
void CompilationEngine::compile_return() {
    _xml_parse_tree->open_xml("returnStatement");

    // return
    xml_capture_token();

    // Optional: expression
    if (_lexical_analyser->try_advance() && _lexical_analyser->get_token() != ";") {
        // There is a return value. We now resolve the expression.
        _lexical_analyser->step_back();
        compile_expression(0);
    } else {
        _lexical_analyser->step_back();
    }

    // ;
    expect_token(";", "Unterminated return statement.");

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
void CompilationEngine::compile_expression(int nest_level) {
    _xml_parse_tree->open_xml("expression");

    // Process the first term, then handle subsequent operators and terms.
    // There must be at least a single term for the expression to be valid.
    compile_term(nest_level);

    std::string curr_token;


    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_token();

        // We expect the next token to be a binary operator, otherwise we stop
        // compiling the expression.
        if (LexicalAnalyser::binary_operators.find(curr_token) != LexicalAnalyser::binary_operators.end()) {
            xml_capture_token();
            compile_term(nest_level);
        } else if (curr_token == ")" && nest_level > 0) {
            // Note: the nest_level is used to indicate whether we are in a 
            //       nested expression or not. If we are then we will 'consume'
            //       the ) character.
            _xml_parse_tree->close_xml();
            xml_capture_token();
            break;
        } else {
            // Take a step back since we looked ahead.
            _lexical_analyser->step_back();
            _xml_parse_tree->close_xml();
            break;
        }
    }
}

void CompilationEngine::compile_term(int nest_level) {
    _xml_parse_tree->open_xml("term");
    _lexical_analyser->try_advance();
    xml_capture_token();

    std::string curr_token = _lexical_analyser->get_token();
    std::string peeked_token;
    TokenType token_type = _lexical_analyser->token_type();


    switch (token_type) {
        case TokenType::KEYWORD:
            if (LexicalAnalyser::builtin_literals.find(curr_token) == LexicalAnalyser::builtin_literals.end())
                throw JackCompilationEngineError(*_lexical_analyser, "Invalid keyword for term '" + curr_token + "'.");
            break;
        case TokenType::IDENTIFIER:
            // We need to look ahead one character to ascertain whether this
            // term is a subroutine call or a reference to a variable.
            peeked_token = _lexical_analyser->peek();
            if (peeked_token == "(" || peeked_token == ".") {
                // Compile subroutine call.
                compile_subroutine_invocation();
            } else if (peeked_token == "[") {
                // Compile subscript operator on variable.
                try_compile_subscript();
            }
            break;
        case TokenType::SYMBOL:
            if (curr_token == "(") {
                // Compile a sub-expression.
                compile_expression(nest_level + 1);
            } else if (LexicalAnalyser::unary_operators.find(curr_token) != LexicalAnalyser::unary_operators.end()) {
                // When a unary operator is encountered, we expect a term to 
                // immediately follow.
                compile_term(nest_level);
            } else {
                throw JackCompilationEngineError(*_lexical_analyser, "Unexpected expression symbol '" + curr_token + "'.");
            }
            break;
        default:
            break;
    }
    _xml_parse_tree->close_xml();
}

void CompilationEngine::compile_subroutine_invocation() {
    _lexical_analyser->try_advance();
    std::string token = _lexical_analyser->get_token();
    xml_capture_token(); // ( or .

    if (token == "(") {
        compile_expression_list();
    } else if (token == ".") {
        // Follow the '.' chain down to the subroutine invocation.
        // Eg. if the token stream consisted of `MyClass.myMethod(...)`, then
        //     the recursive call would process `myMethod(...)`.
        expect_token_type(TokenType::IDENTIFIER, "Expected an identifier in subroutine invocation.");
        compile_subroutine_invocation();
    } else {
        throw JackCompilationEngineError(*_lexical_analyser, "Invalid subroutine invocation.");
    }
}

int CompilationEngine::compile_expression_list() {
    _xml_parse_tree->open_xml("expressionList");
    int num_expressions = 0;

    std::string curr_token;
    while (_lexical_analyser->try_advance()) {
        curr_token = _lexical_analyser->get_token();

        // End expression list compilation when ) is reached.
        if (curr_token == ")") {
            _xml_parse_tree->close_xml();
            xml_capture_token();
            break;
        }
        
        // We expect the next token to be a comma followed by another
        // expression, but only after the first encountered expression.
        if (num_expressions == 0) {
            _lexical_analyser->step_back();
            compile_expression(0);
        } else if (curr_token == ",") {
            xml_capture_token();
            compile_expression(0);
        } else {
            throw JackCompilationEngineError(*_lexical_analyser, "Expected comma between expressions.");
        }

        ++num_expressions;
    }

    return num_expressions;
}

bool CompilationEngine::try_compile_subscript() {
    if (_lexical_analyser->try_advance() && _lexical_analyser->get_token() == "[") {
        xml_capture_token();
        compile_expression(0);
        expect_token("]", "Expected subscript operator terminator, ].");
        return true;
    } else {
        _lexical_analyser->step_back();
        return false;
    }
}

// Trailing variables lists are of the form: `, var1, var2, ...`
bool CompilationEngine::try_compile_trailing_variable_list() {
    bool compiled = false;
    while (_lexical_analyser->try_advance() && _lexical_analyser->get_token() == ",") {
        xml_capture_token();
        expect_token_type(TokenType::IDENTIFIER, "Expected an identifier for field declaration.");
        compiled = true;
    }
    _lexical_analyser->step_back();
    return compiled;
}

std::string CompilationEngine::expect_token_type(TokenType token_type, const std::string& err_message) {
    _lexical_analyser->try_advance();
    if (_lexical_analyser->token_type() != token_type) {
        throw JackCompilationEngineError(*_lexical_analyser, err_message.c_str());
    }
    return xml_capture_token();
}

std::string CompilationEngine::expect_token(const std::string& token, const std::string& err_message) {
    _lexical_analyser->try_advance();
    if (_lexical_analyser->get_token() != token) {
        throw JackCompilationEngineError(*_lexical_analyser, err_message.c_str());
    }
    return xml_capture_token();
}

// A valid data type is either a built-in type or an identifier (it's not the
// responsibility of this function to determine whether it actually references
// a valid class (TODO: yet?)).
std::string CompilationEngine::expect_data_type(const std::string& err_message) {
    _lexical_analyser->try_advance();
    std::string token = _lexical_analyser->get_token();
    if ((LexicalAnalyser::data_types.find(token) == LexicalAnalyser::data_types.end()) &&
            _lexical_analyser->token_type() != TokenType::IDENTIFIER) {
        throw JackCompilationEngineError(*_lexical_analyser, err_message.c_str());
    }
    return xml_capture_token();
}

std::string CompilationEngine::xml_capture_token() {
    std::string token = _lexical_analyser->get_token();
    _xml_parse_tree->form_xml(_lexical_analyser->get_token_type(), token);
    return token;
}

JackCompilationEngineError::JackCompilationEngineError(const std::string& message) throw()
        : _message(message.c_str()) {
    std::cerr << Colour::RED
              << "CompilationEngine Error: " 
              << message
              << Colour::RESET
              << std::endl;
}

JackCompilationEngineError::JackCompilationEngineError(LexicalAnalyser& lexical_analyser, char const* const message) throw() 
        : _message(message) {
    std::cerr << Colour::RED
              << "CompilationEngine Error: " 
              << message
              << Colour::RESET
              << std::endl;
}

JackCompilationEngineError::JackCompilationEngineError(LexicalAnalyser& lexical_analyser, const std::string& message) throw()
        : JackCompilationEngineError(lexical_analyser, message.c_str()) {
}

char const* JackCompilationEngineError::what() const throw() {
    return _message;
}

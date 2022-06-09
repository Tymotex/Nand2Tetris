#include "LexicalAnalyser.h"
#include "CompilationEngine.h"
#include "VMWriter.h"
#include "utils/XMLOutput.h"
#include "utils/Colouriser.h"
#include <iostream>
#include <fstream>
#include <memory>

CompilationEngine::CompilationEngine(
        std::shared_ptr<LexicalAnalyser> lexical_analyser,
        const std::string& class_name,
        std::ostream& vm_stream, std::ostream& xml_stream)
        : _lexical_analyser(lexical_analyser),
          _vm_writer(VMWriter(vm_stream)),
          _xml_parse_tree(std::make_unique<XMLOutput>(xml_stream, true, false)),
          _class_name(class_name),
          _uniq_counter_if(0),
          _uniq_counter_while(0) {
}

CompilationEngine::~CompilationEngine() {
    _xml_parse_tree->close();
}

// Class declarations are of the form:
//     class className { body }
// Note: no code is generated from a class declaration alone.
void CompilationEngine::compile_class() {
    _xml_parse_tree->open_xml("class");

    // Initialise the class-level symbol table to capture new fields.
    _class_symbol_table.reset();

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

    // Record the identifier in the class-level symbol table.
    _class_symbol_table.define(identifier, data_type, decl_type);

    // Optional trailing variable list: ', varName2, varName3, ...'
    try_compile_trailing_variable_list(data_type, decl_type, false);
    
    // ;
    expect_token(";", "Unterminated field declaration.");
    
    _xml_parse_tree->close_xml();
}

// Subroutine declarations are of the form:
//     constructor|function|method type subroutineName (parameterList) { body }
void CompilationEngine::compile_subroutine() {
    _xml_parse_tree->open_xml("subroutineDec");

    // Initialise the subroutine-level symbol table.
    _subroutine_symbol_table.reset();

    // constructor|function|method
    std::string subroutine_decl_type = xml_capture_token();

    // type
    expect_data_type("Expected subroutine return type.");

    // subroutineName
    std::string subroutine_label = _class_name + "." +
        expect_token_type(TokenType::IDENTIFIER, "Expected subroutine name.");

    // (parameterList)
    expect_token("(", "Expected start of parameter list.");
    // Methods always implicitly take in `this` as the first argument, which is
    // also implicitly bound when it's invoked.
    if (subroutine_decl_type == "method")
        _subroutine_symbol_table.define("this", _class_name, "argument");
    compile_parameter_list(); 
    expect_token(")", "Expected end of parameter list.");

    // Begin compilation for subroutine body.
    _xml_parse_tree->open_xml("subroutineBody");

    // { varDeclarations*
    expect_token("{", "Expected start of subroutine scope.");

    while (_lexical_analyser->try_advance() && _lexical_analyser->get_token() == "var")
        compile_variable_declaration();
    _lexical_analyser->step_back();

    // Generate code for declaring the VM function.
    _vm_writer.write_function(subroutine_label, _subroutine_symbol_table.var_count(DeclarationType::VAR));
    if (subroutine_decl_type == "method") {
        // All methods must align `this` upfront. This means that the statements
        // in the method body can execute with the assumption that `this` is
        // bound correctly as a precondition.
        _vm_writer.write_push(VirtualMemorySegment::ARGUMENT, 0);// TODO: duplicated alignment of this
        _vm_writer.write_pop(VirtualMemorySegment::POINTER, 0);
    } else if (subroutine_decl_type == "constructor") {
        // All constructors must allocate the memory for the object and align
        // `this` upfront. The `sizeof` the object is simply the number of
        // fields in the class-level symbol table.
        int mem_size = _class_symbol_table.var_count(DeclarationType::FIELD);
        _vm_writer.write_push(VirtualMemorySegment::CONSTANT, mem_size);
        // TODO: it feels weird hard-coding these OS calls.
        _vm_writer.write_call("Memory.alloc", 1);
        _vm_writer.write_pop(VirtualMemorySegment::POINTER, 0); // TODO: duplicated alignment of this
    }

    // statement* }
    compile_statements();
    expect_token("}", "Expected end of subroutine scope.");

    _xml_parse_tree->close_xml();

    // TODO: if we want to handle implicit `return this;`, it should be done here.

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
        std::string data_type =
            expect_data_type("Expected data type for parameter " + param_num);
        std::string identifier =
            expect_token_type(TokenType::IDENTIFIER, "Expected identifier for parameter " + param_num);

        _subroutine_symbol_table.define(identifier, data_type, "argument");

        ++param_num;
    }
}

// Note: `compile_statements` will terminate when the closing brace character is
//       encountered.
void CompilationEngine::compile_statements() {
    _xml_parse_tree->open_xml("statements");

    std::string curr_token;
    int uniq_counter = 0;
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
    std::string decl_type = xml_capture_token();

    // type
    std::string data_type = expect_data_type("Expected data type for variable declaration.");

    // varName
    std::string identifier = expect_token_type(TokenType::IDENTIFIER, "Expected identifier for variable declaration.");

    // Record the identifier in the symbol table.
    _subroutine_symbol_table.define(identifier, data_type, decl_type);

    // Optional trailing variable list: ', varName2, varName3, ...'
    try_compile_trailing_variable_list(data_type, decl_type, true);

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
    std::string identifier = expect_token_type(TokenType::IDENTIFIER, "Expected variable identifier for let statement");
    if (!is_identifier_defined(identifier))
        throw JackCompilationEngineError("Assignment to undeclared variable, '" + identifier + "'.");
    SymbolTable& symbol_table = get_symbol_table_containing(identifier);

    // Optional: subscript operator, '[expression]'
    bool array_index_assignment = try_compile_subscript();
    // Generate code to access the correct address of `varName[expression]`.
    if (array_index_assignment) {
        // Put the array item's location into the THAT slot.
        // Note: we expect the index expression's value to be on the top of the
        //       stack.
        VirtualMemorySegment segment = // TODO: act of pushing to stack is duplicated
            decl_type_to_segment(symbol_table.declaration_type(identifier));
        int index = symbol_table.segment_index(identifier);

        _vm_writer.write_push(segment, index); 
        _vm_writer.write_arithmetic(ArithmeticLogicOp::ADD);

        // The top of the stack should now contain the memory address of 
        // `varName[expression]`.
    }
    
    // =
    expect_token("=", "Expected assignment operator for let statement");

    // expression
    compile_expression(0);
    // Generate code to dump the expression's value into `temp 0` if we are
    // assigning to an array element.
    if (array_index_assignment) 
        _vm_writer.write_pop(VirtualMemorySegment::TEMP, 0);
    
    // ;
    expect_token(";", "Unterminated let statement.");

    // Generate code to assign `expression` to `varName`. We handle the array
    // index case, `varName[expression]`, separately.
    // NOTE: The expression's evaluted value is expected to be at `temp 0`
    //       for the `varName[expression]` case, otherwise it'll be at the top
    //       of the stack.
    if (!array_index_assignment) {
        VirtualMemorySegment segment = // TODO: act of pushing to stack is duplicated.
            decl_type_to_segment(symbol_table.declaration_type(identifier));
        int index = symbol_table.segment_index(identifier);
        _vm_writer.write_pop(segment, index);
    } else {
        // First, place `varName[expression]`'s address into THAT, then pop
        // whatever was in `temp 0` into `varName[expression]`.
        _vm_writer.write_pop(VirtualMemorySegment::POINTER, 1); 
        _vm_writer.write_push(VirtualMemorySegment::TEMP, 0);
        _vm_writer.write_pop(VirtualMemorySegment::THAT, 0);
    }

    _xml_parse_tree->close_xml();
}

// If-statements can be of the form:
//     if (expression) { statements }
//     if (expression) { statements } else { statements }
void CompilationEngine::compile_if() {
    _xml_parse_tree->open_xml("ifStatement");
    
    std::string if_true_label =
        "IF_TRUE" + std::to_string(_uniq_counter_if);
    std::string if_false_label =
        "IF_FALSE" + std::to_string(_uniq_counter_if);
    std::string end_if_label =
        "IF_END" + std::to_string(_uniq_counter_if++);

    // if
    xml_capture_token();
    
    // (expression)
    expect_token("(", "Expected start of condition for if-statement.");
    compile_expression(0);
    expect_token(")", "Expected end of condition for if-statement.");

    // { statements }
    _vm_writer.write_if(if_true_label);
    _vm_writer.write_goto(if_false_label);
    _vm_writer.write_label(if_true_label);
    expect_token("{", "Expected start of if-statement scope.");
    compile_statements();
    expect_token("}", "Expected end of if-statement scope.");

    // Optional: else { statements }
    _lexical_analyser->try_advance();
    if (_lexical_analyser->get_token() == "else") {
        _vm_writer.write_goto(end_if_label);
        _vm_writer.write_label(if_false_label);
        xml_capture_token();
        expect_token("{", "Expected start of else-statement scope.");
        compile_statements();
        expect_token("}", "Expected end of if-statement scope.");
        _vm_writer.write_label(end_if_label);
    } else {
        _lexical_analyser->step_back();
        _vm_writer.write_label(if_false_label);
    }

    _xml_parse_tree->close_xml();
}

// While statements can be of form:
//     while (expression) { statements }
void CompilationEngine::compile_while() {
    _xml_parse_tree->open_xml("whileStatement");

    std::string start_loop_label =
        "WHILE_EXP" + std::to_string(_uniq_counter_while);
    std::string end_loop_label =
        "WHILE_END" + std::to_string(_uniq_counter_while++);
    
    // while
    xml_capture_token();
    _vm_writer.write_label(start_loop_label);

    // (expression)
    expect_token("(", "Expected start of condition for while-loop.");
    compile_expression(0);
    expect_token(")", "Expected end of condition for while-loop.");
    _vm_writer.write_arithmetic(ArithmeticLogicOp::NOT);
    _vm_writer.write_if(end_loop_label);

    // { statements }
    expect_token("{", "Expected start of while-loop scope.");
    compile_statements();
    expect_token("}", "Expected end of while-loop scope.");
    _vm_writer.write_goto(start_loop_label);
    _vm_writer.write_label(end_loop_label);

    _xml_parse_tree->close_xml();
}

// Do statements are of the form:
//     do subroutineInvocation;
void CompilationEngine::compile_do() {
    // In Jack, a subroutine call is only ever present in `do` statements and
    // in expressions. Here, we are reusing the `compile_expression` algorithm
    // to compile direct subroutine calls of the form `do subroutine_call()`.
    _xml_parse_tree->open_xml("doStatement");
    
    // do
    xml_capture_token();
    
    // subroutineInvocation
    std::string identifier = expect_token_type(TokenType::IDENTIFIER, "Expected identifier for subroutine invocation in do statement.");
    compile_subroutine_invocation(identifier);

    // ;
    expect_token(";", "Unterminated do statement.");
    
    // Generate code to discard the value at the top of the stack. We do this
    // because the `do` statement is used for its side effect, not its value.
    _vm_writer.write_pop(VirtualMemorySegment::TEMP, 0);
    
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
    bool has_ret_val = false;
    if (_lexical_analyser->try_advance() && _lexical_analyser->get_token() != ";") {
        // There is a return value. We now resolve the expression.
        _lexical_analyser->step_back();
        compile_expression(0);
        has_ret_val = true;
    } else {
        _lexical_analyser->step_back();
    }

    // ;
    expect_token(";", "Unterminated return statement.");

    // Generate code to terminate the subroutine. If no value was returned, we
    // must still push some value to the stack. It'll simply be discarded.
    if (!has_ret_val) _vm_writer.write_push(VirtualMemorySegment::CONSTANT, 0);
    _vm_writer.write_return();

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

            // Note: the VM language specification does not include multiplication
            //       and division. These are implemented by the OS.
            if (curr_token == "*") {
                _vm_writer.write_call("Math.multiply", 2);
            } else if (curr_token == "/") {
                _vm_writer.write_call("Math.divide", 2);
            } else {
                _vm_writer.write_arithmetic(curr_token);
            }
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

// A term can be one of the following sets:
// - Built-in identifiers: { true, false, this, null } 
// - An integer constant like `42` or string constant like "hello"
// - A local variable/argument or instance/static class variable
// - A subroutine invocation.
// - An array element accessed with the subscript operator
// - A unary operator acting on a sub-term, eg. `-1` or `~func(2, 3)`
// - A sub-expression, eg. `(1 + 2)`
void CompilationEngine::compile_term(int nest_level) {
    _xml_parse_tree->open_xml("term");
    _lexical_analyser->try_advance();

    xml_capture_token();

    std::string curr_token = _lexical_analyser->get_token();
    std::string peeked_token;
    TokenType token_type = _lexical_analyser->token_type();

    switch (token_type) {
        case TokenType::KEYWORD:
            // Lookup built-in literals.
            if (LexicalAnalyser::builtin_literals.find(curr_token) == LexicalAnalyser::builtin_literals.end())
                throw JackCompilationEngineError(*_lexical_analyser, "Invalid keyword for term '" + curr_token + "'.");

            if (curr_token == "true") {
                // Pushing 1 then negating will produce -1: 1111111111111111.
                _vm_writer.write_push(VirtualMemorySegment::CONSTANT, 0);
                _vm_writer.write_arithmetic(ArithmeticLogicOp::NOT);
            } else if (curr_token == "false")
                _vm_writer.write_push(VirtualMemorySegment::CONSTANT, 0);
            else if (curr_token == "null")
                _vm_writer.write_push(VirtualMemorySegment::CONSTANT, 0);
            else if (curr_token == "this")
                _vm_writer.write_push(VirtualMemorySegment::POINTER, 0);
            
            break;
        case TokenType::IDENTIFIER:
            // We need to look ahead one character to ascertain whether this
            // term is a subroutine call, a reference to an array element, 
            // or a reference to a variable.
            peeked_token = _lexical_analyser->peek();
            if (peeked_token == "(" || peeked_token == ".")
                compile_subroutine_invocation(curr_token);
            else if (peeked_token == "[") {
                try_compile_subscript();
                
                // TODO: this might be duplicated in compile_let.
                // Put the array item's location into the THAT slot, then 
                // look up its contents to then push onto the stack.
                // Note: we expect the index expression's value to be on the top of the
                //       stack.
                SymbolTable& symbol_table = get_symbol_table_containing(curr_token); // TODO: act of pushing is duplicated
                VirtualMemorySegment segment =
                    decl_type_to_segment(symbol_table.declaration_type(curr_token));
                int index = symbol_table.segment_index(curr_token);

                _vm_writer.write_push(segment, index); 
                _vm_writer.write_arithmetic(ArithmeticLogicOp::ADD);

                _vm_writer.write_pop(VirtualMemorySegment::POINTER, 1);
                _vm_writer.write_push(VirtualMemorySegment::THAT, 0);
            } else {
                SymbolTable& symbol_table = get_symbol_table_containing(curr_token); // TODO: the act of pushing a variable onto the stack is duplicated quite a lot.
                VirtualMemorySegment segment = decl_type_to_segment(symbol_table.declaration_type(curr_token));
                int index = symbol_table.segment_index(curr_token);
                _vm_writer.write_push(segment, index);
            }
            break;
        case TokenType::SYMBOL:
            if (curr_token == "(") {
                // This term is a sub-expression with further terms.
                compile_expression(nest_level + 1);
            } else if (LexicalAnalyser::unary_operators.find(curr_token) != LexicalAnalyser::unary_operators.end()) {
                // When a unary operator is encountered, we expect a term to 
                // immediately follow. Note that `compile_term` can also
                // recursively resolve sub-expressions.
                compile_term(nest_level);
                if (curr_token == "-")
                    _vm_writer.write_arithmetic(ArithmeticLogicOp::NEG);
                else if (curr_token == "~")
                    _vm_writer.write_arithmetic(ArithmeticLogicOp::NOT);
            } else {
                throw JackCompilationEngineError(*_lexical_analyser, "Unexpected expression symbol '" + curr_token + "'.");
            }
            break;
        case TokenType::INT_CONST:
            _vm_writer.write_push(VirtualMemorySegment::CONSTANT, std::stoi(curr_token));
            break;
        case TokenType::STRING_CONST:
            construct_string(curr_token);
            break;
        default:
            throw JackCompilationEngineError(*_lexical_analyser, "Unexpected token '" + curr_token + "'.");
            break;
    }
    _xml_parse_tree->close_xml();
}

void CompilationEngine::compile_subroutine_invocation_recursive(
        const std::string& first_token, const std::string& subroutine_name, int depth) {

}

void CompilationEngine::compile_subroutine_invocation(
        const std::string& first_token) {
    _lexical_analyser->try_advance();

    // ( or .
    std::string next_token = _lexical_analyser->get_token();
    xml_capture_token(); 

    if (next_token == "(") {
        // If the fully qualified subroutine name is only 1 word long, then
        // assume that the method is to be invoked on `this`.
        _vm_writer.write_push(VirtualMemorySegment::POINTER, 0);
        int num_args = compile_expression_list();
        _vm_writer.write_call(_class_name + "." + first_token,
            num_args + 1);
    } else if (next_token == ".") {
        // Follow the '.' chain down to the subroutine invocation. The first
        // part of the fully qualified subroutine invocation is the class_name
        std::string suffix = expect_token_type(TokenType::IDENTIFIER, "Expected an identifier in subroutine invocation.");
        expect_token("(", "Expected the start of a parameter list");

        // If the `first_token` is not an identifier, then we assume that it
        // refers to a static function, otherwise it's a method invocation.
        if (is_identifier_defined(first_token)) {
            const std::string& obj = first_token;
            SymbolTable& symbol_table = get_symbol_table_containing(obj);
            const std::string& class_name = symbol_table.data_type(obj);
            VirtualMemorySegment segment = decl_type_to_segment(symbol_table.declaration_type(obj)); // TODO: duplicated push
            int index = symbol_table.segment_index(obj);
            _vm_writer.write_push(segment, index);
            int num_args = compile_expression_list();
            _vm_writer.write_call(class_name + "." + suffix, num_args + 1);
        } else {
            const std::string& class_name = first_token;
            int num_args = compile_expression_list();
            _vm_writer.write_call(class_name + "." + suffix, num_args);
        }
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
        } else if (curr_token == ",") {
            xml_capture_token();
        } else {
            throw JackCompilationEngineError(*_lexical_analyser, "Expected comma between expressions.");
        }
        compile_expression(0);

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
bool CompilationEngine::try_compile_trailing_variable_list(
        const std::string data_type, const std::string& decl_type,
        const bool is_subroutine_scope) {
    bool compiled = false;
    while (_lexical_analyser->try_advance() && _lexical_analyser->get_token() == ",") {
        xml_capture_token();
        std::string identifier = expect_token_type(TokenType::IDENTIFIER, "Expected an identifier for field declaration.");
        compiled = true;

        // Record the identifier in the class-level symbol table.
        SymbolTable& symbol_table = (is_subroutine_scope) ?
            _subroutine_symbol_table : _class_symbol_table;
        symbol_table.define(identifier, data_type, decl_type);
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

bool CompilationEngine::is_identifier_defined(const std::string& identifier) {
    return _class_symbol_table.exists(identifier) ||
        _subroutine_symbol_table.exists(identifier);
}

SymbolTable& CompilationEngine::get_symbol_table_containing(const std::string& identifier) {
    if (_subroutine_symbol_table.exists(identifier)) return _subroutine_symbol_table;
    else if (_class_symbol_table.exists(identifier)) return _class_symbol_table;
    throw JackCompilationEngineError("Expected '" + identifier + "' to be in scope.");
}

VirtualMemorySegment CompilationEngine::decl_type_to_segment(const DeclarationType decl_type) {
    switch (decl_type) {
        case DeclarationType::FIELD:
            return VirtualMemorySegment::THIS;
        case DeclarationType::STATIC:
            return VirtualMemorySegment::STATIC;
        case DeclarationType::VAR:
            return VirtualMemorySegment::LOCAL;
        case DeclarationType::ARGUMENT:
            return VirtualMemorySegment::ARGUMENT;
    }
    throw JackCompilationEngineError("Unknown declaration type.");
}

void CompilationEngine::construct_string(const std::string& s) {
    _vm_writer.write_push(VirtualMemorySegment::CONSTANT, s.size());
    _vm_writer.write_call("String.new", 1);

    // To initialise the characters of the constructed string, we need to 
    // repeatedly invoke the OS subroutine, `String.appendChar`.
    for (const char& c : s) {
        _vm_writer.write_push(VirtualMemorySegment::CONSTANT, c);
        _vm_writer.write_call("String.appendChar", 2);
        // Note: there's no need to repush `this` because `String.append` will
        //       return it, leaving it at the top of the stack as input into the
        //       next round of append.
    }
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

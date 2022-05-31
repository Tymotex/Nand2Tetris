#include "LexicalAnalyser.h"
#include "Parser.h"
#include "utils/XMLOutput.h"
#include <iostream>
#include <fstream>
#include <memory>

Parser::Parser(std::shared_ptr<LexicalAnalyser> lexical_analyser, const std::string& output_stream)
    : _lexical_analyser(lexical_analyser),
      _xml_parse_tree(std::make_unique<XMLOutput>(output_stream, true)) {
}

void Parser::compile_class() {
}

void Parser::compile_subroutine() {

}

void Parser::compile_subroutine_body() {

}

void Parser::compile_variable_declaration() {

}

void Parser::compile_statements() {

}

void Parser::compile_let() {

}

void Parser::compile_if() {

}

void Parser::compile_while() {

}

void Parser::compile_do() {

}

void Parser::compile_return() {

}

void Parser::compile_expression() {

}

void Parser::compile_term() {

}

void Parser::compile_class_body() {

}

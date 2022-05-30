// TODO: for subroutineCall, see page 276.

#include "LexicalAnalyser.h"
#include "Parser.h"
#include <iostream>
#include <fstream>

Parser::Parser(LexicalAnalyser& token_stream, std::ofstream& output_stream)
    : _token_stream(token_stream) {
}

void Parser::compile_class() {
}

void Parser::compile_member_variable_declaration() {
}

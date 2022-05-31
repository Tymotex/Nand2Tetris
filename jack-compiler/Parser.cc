#include "LexicalAnalyser.h"
#include "Parser.h"
#include <iostream>
#include <fstream>
#include <memory>

Parser::Parser(std::shared_ptr<LexicalAnalyser> lexical_analyser, std::ofstream& output_stream)
    : _lexical_analyser(lexical_analyser) {
}

void Parser::compile_class() {
}


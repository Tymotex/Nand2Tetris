#include "VMParser.h"
#include <fstream>
#include <iostream>
#include <regex>

VMParser::VMParser(const std::string& vm_source_file_path) 
    : _vm_in(std::ifstream(vm_source_file_path)) {
}

bool VMParser::has_more_lines() {
    return !_vm_in.eof();
}

void VMParser::advance() {
    std::string line;
    if (!std::getline(_vm_in, line)) return;

    if ()

}

VMOperationType VMParser::command_type() {
    return _command_type;
}

std::string VMParser::arg1() {
    return _arg1;
}

int VMParser::arg2() {
    return _arg2;
}

void VMParser::preprocess() {
    // Strips all leading and trailing whitespaces.
    std::string whitespaces = " \t";
    int start_index = _curr_line.find_first_not_of(whitespaces);
    int last_index = _curr_line.find_last_not_of(whitespaces);
    
    int run_len = (last_index + 1) - start_index;
    _curr_line = _curr_line.substr(start_index, run_len);

    // Strip inline comments. TODO:
}

bool VMParser::parse() {
    preprocess();
    
    // First, we determine what command type and therefore what subsequent
    // arguments to expect.
    int run_len = 0; 
    while (run_len < _curr_line.size()) {
        const char& c = _curr_line[run_len];
        if (c == ' ') break;
        ++run_len;
    }
    std::string instruction = _curr_line.substr(0, run_len);

    // Next, we pull out the expected arguments and populate/clear _arg1 and
    // _arg2.
    if (instruction == "push" || instruction == "pop") {
        // Expect the segment name, followed by the index.
        std::regex push_pop_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+)$)");
        std::smatch matches;
        if (!std::regex_search(_curr_line, matches, push_pop_pattern)) {
            std::cerr << "Syntax Error: push/pop commands expect 2 args.\n";
        _arg1 = matches[1];
        _arg2 = stoi(matches[2]);
        _command_type = VMOperationType::C_POP;
    } else if (instruction == "add" || instruction == "sub" || instruction == "and" || instruction == "or" || instruction == "eq" || instruction == "gt" || instruction == "lt" || instruction == "neg" || instruction == "not") {
        // If additional arguments were supplied, then the instruction is invalid.
        std::regex arithmetic_logic_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+)$)");
        std::smatch matches;
        if (!std::regex_search(_curr_line, matches, arithmetic_logic_pattern)) {
            std::cerr << "Syntax Error: arithmetic/logical instructions expect no arguments.\n";
            return false;
        }

        // Clear _arg1 and _arg2.
        _arg1 = "";
        _arg2 = -1;
        _command_type = VMOperationType::C_ARITHMETIC;
    } else {
        // Clear _arg1 and _arg2.
        _arg1 = "";
        _arg2 = -1;
        _command_type = VMOperationType::INVALID;
        return false;
    }
    return true;
}

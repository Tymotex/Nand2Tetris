#include "VMParser.h"
#include <fstream>
#include <iostream>
#include <regex>

VMParser::VMParser(const std::string& vm_source_file_path) 
    : _vm_in(std::ifstream(vm_source_file_path)),
      _curr_line(0) {
}

bool VMParser::has_more_lines() {
    return !_vm_in.eof();
}

void VMParser::advance() {
    if (!std::getline(_vm_in, _curr_instruction)) {
        _command_type = VMOperationType::INVALID;
        _arg1 = "";
        _arg2 = -1;
        return;
    }
    ++_curr_line;
    if (!parse()) advance();
    else show_instruction_debug_info();
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

std::string VMParser::get_curr_instruction() {
    return _curr_instruction;
}

void VMParser::preprocess() {
    // Strips all leading whitespace.
    const std::string WHITESPACE = " \n\r\t\f\v";

    int start_index = _curr_instruction.find_first_not_of(WHITESPACE);
    if (start_index != std::string::npos) _curr_instruction = _curr_instruction.substr(start_index);
    
    // Strip inline comments.
    int comment_start_index = _curr_instruction.find_first_of("/");
    if (comment_start_index != std::string::npos) _curr_instruction = _curr_instruction.substr(0, comment_start_index);

    // Strip all trailing whitespace.
    int last_index = _curr_instruction.find_last_not_of(WHITESPACE);
    if (last_index != std::string::npos) _curr_instruction = _curr_instruction.substr(0, last_index + 1);
}

bool VMParser::parse() {
    preprocess();
    if (_curr_instruction.empty()) return false;
    
    // First, we determine what command type and therefore what subsequent
    // arguments to expect.
    int run_len = 0; 
    while (run_len < _curr_instruction.size()) {
        const char& c = _curr_instruction[run_len];
        if (c == ' ') break;
        ++run_len;
    }
    std::string instruction = _curr_instruction.substr(0, run_len);

    // Next, we pull out the expected arguments and populate/clear _arg1 and
    // _arg2.
    // TODO: replace if statements with a hash table lookup?
    if (instruction == "push") {
        // Expect the segment name, followed by the index.
        std::regex push_pop_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, push_pop_pattern)) {
            std::cerr << "Syntax Error: push/pop commands expect 2 args.\n";
            return false;
        }
        _arg1 = matches[2];
        _arg2 = stoi(matches[3]);
        _command_type = VMOperationType::C_PUSH;
    } else if (instruction == "pop") {
        // Expect the segment name, followed by the index.
        std::regex push_pop_pattern(R"(^(push|pop) +([a-zA-Z]+) +([0-9]+))");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, push_pop_pattern)) {
            std::cerr << "Syntax Error: push/pop commands expect 2 args.\n";
            return false;
        }
        _arg1 = matches[2];
        _arg2 = stoi(matches[3]);
        _command_type = VMOperationType::C_POP;
    } else if (instruction == "add" || instruction == "sub" || instruction == "and" || instruction == "or" || instruction == "eq" || instruction == "gt" || instruction == "lt" || instruction == "neg" || instruction == "not") {
        // If additional arguments were supplied, then the instruction is invalid.
        std::regex arithmetic_logic_pattern(R"(^[a-zA-Z]+)");
        std::smatch matches;
        if (!std::regex_search(_curr_instruction, matches, arithmetic_logic_pattern)) {
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

void VMParser::show_instruction_debug_info() {
    std::cout << "Line " << _curr_line << ") " << _curr_instruction << "\n";
    switch (_command_type) {
        case VMOperationType::C_ARITHMETIC:
            std::cout << "\tInstruction: arithmetic" << "\n";
            break;
        case VMOperationType::C_PUSH:
            std::cout << "\tInstruction: push" << "\n";
            break;
        case VMOperationType::C_POP:
            std::cout << "\tInstruction: pop" << "\n";
            break;
        case VMOperationType::INVALID:
            std::cout << "\tInstruction: INVALID";
            break;
        default:
            break;
    }
    std::cout << "\tArg1: " << _arg1 << "\n";
    std::cout << "\tArg2: " << _arg2 << "\n";
}

#include "Parser.h"
#include <iostream>
#include <regex>

HackAsmParser::HackAsmParser(std::string asm_source_file_path)
        : _asm_file(std::ifstream(asm_source_file_path)),
          _curr_line_num(0) {}

bool HackAsmParser::has_more_lines() {
    return !_asm_file.eof();
}

bool HackAsmParser::advance() {
    if (!has_more_lines()) return false;
    
    // Get next line, if it's an instruction, then stop, otherwise continue
    // advancing until a valid instruction is encountered or EOF.
    ++_curr_line_num;
    std::getline(_asm_file, _curr_instruction);
    normalise();
    parse();
    // show_curr_instruction_info();
    if (instruction_type() != InstructionType::INVALID_INSTRUCTION) return true;
    return advance();
}

void HackAsmParser::normalise() {
    // Strip all whitespaces.
    std::regex pattern("\\s+");
    _curr_instruction = std::regex_replace(_curr_instruction, pattern, "");

    // Strip trailing inline comment.
    std::regex comment_pattern(R"(//(.*)$)");
    _curr_instruction = std::regex_replace(_curr_instruction, comment_pattern, "");
}

void HackAsmParser::parse() {
    std::string C_INSTRUCTION_REGEX_PATTERN = R"(^(^((M|D|DM|MD|A|AM|MA|AD|DA|ADM|AMD|MAD|MDA|DAM|DMA)=)?(0|1|(D|A|M)?-1|-?(D|A|M)|![DAM]|[DAM]\+1|D[+-][AM]|[AM]-D|D[|&][AM]);?(JGT|JEQ|JGE|JLT|JNE|JLE|JMP)?$))";
    // Any line starting with @ is treated as an A-instruction.
    if (_curr_instruction[0] == '@') {
        _instr_type = InstructionType::A_INSTRUCTION;
    }
    // Any line starting with ( and ending with ) is interpreted as a label
    // declaration.
    else if (_curr_instruction.front() == '(' && _curr_instruction.back() == ')') {
        _instr_type = InstructionType::L_INSTRUCTION;
    }
    // C-instructions will pass the regex check
    else if (std::regex_match(_curr_instruction, std::regex(C_INSTRUCTION_REGEX_PATTERN))) {
        _instr_type = InstructionType::C_INSTRUCTION;
    }
    // Any line starting with // is interpreted as a comment
    else if (_curr_instruction[0] == '/' && _curr_instruction[1] == '/') {
        _instr_type = InstructionType::COMMENT;
    }
    else if (std::all_of(_curr_instruction.begin(), _curr_instruction.end(), [](const char& c){ return isspace(c);})) {
        _instr_type = InstructionType::EMPTY;
    }
    // Any other line is treated as a non-instruction. Report an invalid line.
    else {
        std::cerr << "Syntax Error: at line " << _curr_line_num << ", '" << _curr_instruction << "' is not a valid Hack assembly instruction\n";
        _instr_type = InstructionType::INVALID_INSTRUCTION;
    }
}

InstructionType HackAsmParser::instruction_type() {
    return _instr_type;
}

std::string HackAsmParser::symbol() {
    if (instruction_type() != InstructionType::A_INSTRUCTION && instruction_type() != InstructionType::L_INSTRUCTION) 
        std::cerr << "Assembler Error: Cannot extract symbol from an instruction that is not an A-instruction or L-instruction.\n";
    // Extract from A-instruction:
    std::regex a_instr_pattern(R"(^@(.*)$)");
    std::regex l_instr_pattern(R"(^\((.*)\)$)");
    std::smatch matches;
    if (std::regex_search(_curr_instruction, matches, a_instr_pattern)) {
        return matches[1];
    } else if (std::regex_search(_curr_instruction, matches, l_instr_pattern)) {
        return matches[1];
    }
    std::cerr << "Syntax Error: No symbol found.";
    return "";
}

std::string HackAsmParser::dest() {
    if (instruction_type() != InstructionType::C_INSTRUCTION)
        std::cerr << "Assembler Error: Cannot extract dest from non C-instruction.\n";
    std::regex pattern(R"(^(.*)=(.*);?(.*)?$)");
    std::smatch matches;
    if (!std::regex_search(_curr_instruction, matches, pattern)) {
        return "";
    }
    return matches[1];
}

std::string HackAsmParser::comp() {
    if (instruction_type() != InstructionType::C_INSTRUCTION)
        std::cerr << "Assembler Error: Cannot extract comp from non C-instruction.\n";
    std::regex full_op_pattern(R"(^.*=([-+ADM!&|10]+);?(JGT|JEQ|JGE|JLT|JNE|JLE|JMP)?$)");
    std::regex jmp_only_pattern(R"(^([-+ADM!&|10]+);?(JGT|JEQ|JGE|JLT|JNE|JLE|JMP)?$)");
    std::smatch matches;
    if (std::regex_search(_curr_instruction, matches, full_op_pattern)) {
        return matches[1];
    } else if (std::regex_search(_curr_instruction, matches, jmp_only_pattern)) {
        return matches[1];
    }
    return _curr_instruction;
}

std::string HackAsmParser::jump() {
    if (instruction_type() != InstructionType::C_INSTRUCTION)
        std::cerr << "Assembler Error: Cannot extract jump from non C-instruction.\n";
    std::regex pattern(R"(^.*;(JGT|JEQ|JGE|JLT|JNE|JLE|JMP)$)");
    std::smatch matches;
    if (!std::regex_search(_curr_instruction, matches, pattern)) {
        return "";
    }
    return matches[1];
}

void HackAsmParser::show_curr_instruction_info() {
    std::cout << "Line " << _curr_line_num << ": " << _curr_instruction << "\n";
    switch (_instr_type) {
        case A_INSTRUCTION:
            std::cout << "\tSymb: " << symbol() << "\n";
            break;
        case C_INSTRUCTION:
            std::cout << "\tDest: " << dest() << "\n";
            std::cout << "\tComp: " << comp() << "\n";
            std::cout << "\tJump: " << jump() << "\n";
            break;
        case L_INSTRUCTION:
            std::cout << "\tLabl: " << symbol() << "\n";
            break;
    }
}

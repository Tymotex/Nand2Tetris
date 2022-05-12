#include <iostream>
#include "Code.h"

std::unordered_map<std::string, std::string> MachineCodeMapper::dest_to_code = {
    {"M", "001"},
    {"D", "010"},
    {"DM", "011"},
    {"A", "100"},
    {"AM", "101"},
    {"AD", "110"},
    {"ADM", "111"},
};

std::unordered_map<std::string, std::string> MachineCodeMapper::comp_to_code = {
    {"0", "0101010"},
    {"1", "0111111"},
    {"-1", "0111010"},
    {"D", "0001100"},
    {"A", "0110000"}, {"M", "1110000"},
    {"!D", "0001101"},
    {"!A", "0110011"}, {"!M", "1110011"},
    {"-D", "0001111"},
    {"-A", "0110011"}, {"-M", "1110011"},
    {"D+1", "0011111"},
    {"A+1", "0110111"}, {"M+1", "1110111"},
    {"D-1", "0001110"},
    {"A-1", "0110010"}, {"M-1", "1110010"},
    {"D+A", "0000010"}, {"D+M", "1000010"},
    {"D-A", "0010011"}, {"D-M", "1010011"},
    {"A-D", "0000111"}, {"M-D", "1000111"},
    {"D|A", "0010101"}, {"D|M", "1010101"}
};

std::unordered_map<std::string, std::string> MachineCodeMapper::jump_to_code = {
    {"JGT", "001"},
    {"JEQ", "010"},
    {"JGE", "011"},
    {"JLT", "100"},
    {"JNE", "101"},
    {"JLE", "110"},
    {"JMP", "111"},
};

std::string MachineCodeMapper::dest(std::string asm_code) {
    if (asm_code.empty()) return "000";
    if (dest_to_code.find(asm_code) == dest_to_code.end()) return "000";
    return dest_to_code[asm_code];
}

std::string MachineCodeMapper::comp(std::string asm_code) {
    if (asm_code.empty()) {
        std::cerr << "Assembly Error: No comp code was supplied.\n";
        return "0000000";
    }
    if (comp_to_code.find(asm_code) == comp_to_code.end()) {
        std::cerr << "Assembly Error: Invalid comp code was supplied.\n";
        return "000000";
    }
    return comp_to_code[asm_code];
}

std::string MachineCodeMapper::jump(std::string asm_code) {
    if (asm_code.empty()) return "000";
    if (jump_to_code.find(asm_code) == jump_to_code.end()) return "000";
    return jump_to_code[asm_code];
}

inline bool is_integer(const std::string & s) {
   if (s.empty() || ((!isdigit(s[0])) && (s[0] != '-') && (s[0] != '+'))) return false;
   char* p;
   strtol(s.c_str(), &p, 10);
   return (*p == 0);
}

std::string get_binary_representation(int val) {
    std::string binary_rep = "";
    while (val > 0) {
        binary_rep.insert(binary_rep.begin(), static_cast<char>('0' + val % 2));
        val = val / 2;
    }
    while (binary_rep.size() < 15) binary_rep.insert(binary_rep.begin(), '0');
    return binary_rep;
}

std::string MachineCodeMapper::get_address(std::string asm_code) {
    if (is_integer(asm_code)) {
        int numeric_address = stoi(asm_code);
        return get_binary_representation(numeric_address);
    } else {
        // Convert to numeric address with symbol table, then get binary
        // representation.
        return "TMP";
    }
}

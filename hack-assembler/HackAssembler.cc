#include "Code.h"
#include "Parser.h"
#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <regex>

int main(int argc, char* argv[]) {
    std::string asm_filename;
    if (argc <= 1) {
        std::cout << "Insufficient arguments. Please supply a file with filename *.asm.\n";
        std::cin >> asm_filename;
    } else if (argc == 2) {
        asm_filename = std::string(argv[1]);
    } else if (argc >= 3) {
        std::cerr << "Too many arguments. Please supply exactly one file with filename *.asm.\n";
    }

    if (!std::regex_match(asm_filename, std::regex(".*\\.asm$"))) {
        std::cerr << "Supplied file must be an assembly file with extension .asm\n";
        return 1;
    }

    // Second pass:
    // For each line of assembly instruction (either C-instruction or 
    // A-instruction), map it to the corresponding string of 0s and 1s, then
    // write it to a file.
    MachineCodeMapper code_mapper;
    HackAsmParser parser(asm_filename);
    std::ofstream hack_file("out.hack");
    while (parser.has_more_lines()) {
        parser.advance();

        std::string machine_code = "";
        switch (parser.instruction_type()) {
            case InstructionType::A_INSTRUCTION:
                machine_code += "0";
                machine_code += code_mapper.get_address(parser.symbol());
                hack_file << machine_code << "\n";
                break;
            case InstructionType::C_INSTRUCTION:
                machine_code += "111";
                machine_code += code_mapper.dest(parser.dest());
                machine_code += code_mapper.comp(parser.comp());
                machine_code += code_mapper.jump(parser.jump());
                hack_file << machine_code << "\n";
                break;
            case InstructionType::L_INSTRUCTION:
                break;
        }
    }
    hack_file.close();


    return 0;
}

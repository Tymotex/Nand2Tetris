#include "Code.h"
#include "Parser.h"
#include "SymbolTable.h"
#include <iostream>
#include <string>
#include <regex>

std::string get_basename(std::string path) {
    std::regex filename_pattern(R"(^(.*)\..*$)");
    std::smatch matches;
    if (!std::regex_search(path, matches, filename_pattern))
        std::cerr << "Filename doesn't have a valid name.\n";
    std::string filename = matches[1];
    std::string basename = filename.substr(filename.find_last_of("/\\") + 1);
    return basename;
}

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

    MachineCodeMapper code_mapper;
    HackAsmParser parser(asm_filename);
    SymbolTable symbol_table;
    // First pass:
    // Identify all L-instructions and A-instructions that reference a symbol.
    // Adds the symbols to a symbol table and assigns it an address starting
    // from 16 onwards. 
    int line_num = 0;
    while (parser.has_more_lines()) {
        parser.advance();
        InstructionType instr_type = parser.instruction_type();
        if (instr_type == InstructionType::L_INSTRUCTION) {
            // Add a new symbol entry and point it to the current line number,
            // which is an instruction memory address.
            std::string symbol = parser.symbol();
            if (!symbol_table.contains(symbol)) symbol_table.add_entry(symbol, line_num);
        } else if (instr_type == InstructionType::C_INSTRUCTION || instr_type == InstructionType::A_INSTRUCTION) {
            // Do nothing. Progress the line number.
            ++line_num;
        }
    }

    // Second pass:
    // For each line of assembly instruction (either C-instruction or 
    // A-instruction), map it to the corresponding string of 0s and 1s, then
    // write it to a file.
    std::ofstream hack_file(get_basename(asm_filename) + ".hack");
    parser = HackAsmParser(asm_filename);
    int next_free_data_addr = 16;
    while (parser.has_more_lines()) {
        parser.advance();

        std::string machine_code = "";
        switch (parser.instruction_type()) {
            case InstructionType::A_INSTRUCTION:
                {
                    machine_code += "0";
                    // Add a new symbol entry and point it to the next available slot in
                    // in general RAM.
                    std::string symbol = parser.symbol();
                    if (!code_mapper.is_integer(symbol) && !symbol_table.contains(symbol))
                        symbol_table.add_entry(symbol, next_free_data_addr++);
                    machine_code += code_mapper.get_address(parser.symbol(), symbol_table);
                    hack_file << machine_code << "\n";
                }
                break;
            case InstructionType::C_INSTRUCTION:
                machine_code += "111";
                machine_code += code_mapper.comp(parser.comp());
                machine_code += code_mapper.dest(parser.dest());
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

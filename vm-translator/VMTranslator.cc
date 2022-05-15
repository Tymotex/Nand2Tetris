#include "AsmMapper.h"
#include "VMParser.h"
#include <iostream>
#include <regex>
#include <fstream>

std::string get_basename(const std::string& path) {
    // Locate the last instance of / in the given path, if it exists and use
    // that as the starting index to extract the basename.
    int start_index = path.find_last_of("/");
    if (start_index == std::string::npos) {
        start_index = -1;
    }
    std::string filename = path.substr(start_index + 1);
    std::regex basename_pattern(R"(^(.*)\.vm$)");
    std::smatch matches;
    if (!std::regex_search(filename, matches, basename_pattern)) throw std::invalid_argument("Given filename does not have .vm extension.");
    return matches[1];
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Insufficient arguments. Please supply a path to the .vm source file.\n";
    } else if (argc > 3) {
        std::cerr << "Too many arguments.\n";
    }
    std::string output_file_path = get_basename("hello/world/fuck.vm") + ".asm";
    

    std::ofstream output_asm_file(output_file_path);


    output_asm_file.close();
    return 0;
}
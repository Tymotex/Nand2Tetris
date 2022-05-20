#include "AsmMapper.h"
#include "VMParser.h"
#include <iostream>
#include <regex>
#include <fstream>
#include <filesystem>

// Reads in the VM instructions in the given .vm file and produces an .asm file
// containing all the translated Hack assembly instructions.
// Assumes that the given .vm file exists.
void translate_vm_file_to_asm_file(const std::string& path, const std::string& output_dir);

// Extracts the basename from a given path.
// Eg. Given "/home/linus/hello.txt", `get_basename` returns "hello".
std::string get_basename(const std::string& path);

// Determines whether the given file has the .vm file extension.
bool is_vm_file(const std::string& path);

// Returns the same path that was given, but one level back.
// Eg. Given "/home/linus/hello.txt", `get_directory_of_file` returns "/home/linus".
std::string get_directory_of_file(const std::string& path);

int main(int argc, char* argv[]) {
    if (argc < 2) std::cerr << "Insufficient arguments. Please supply a path to the .vm source file.\n";
    else if (argc > 3) std::cerr << "Too many arguments.\n";

    std::string input_file_path = argv[1];

    // For single-file translation, the output directory will be the same 
    // directory where the source file is. For multi-file translation, an output
    // directory will be created to store all the translated .asm files.
    std::string output_dir = get_directory_of_file(input_file_path);


    // If the given path points to a directory, then process all .vm files found
    // in that directory. Otherwise, process a single file.
    if (std::filesystem::is_directory(input_file_path)) {
        std::cout << argv[0] << ": Translating all .vm files in the given directory.\n\n";

        // Make output directory for the output .asm files.
        output_dir += get_basename(input_file_path) + "_TRANSLATED/";
        std::filesystem::create_directory(output_dir);

        // Translate each .vm file in the given directory.
        for (const std::filesystem::directory_entry& each_file : std::filesystem::directory_iterator(input_file_path)) {
            std::cout << argv[0] << ": Processing " << each_file.path() << "\n";
            if (is_vm_file(each_file.path()))
                translate_vm_file_to_asm_file(each_file.path(), output_dir);
        }
    } else {
        std::cout << argv[0] << ": Translating a single file.\n\n";
        translate_vm_file_to_asm_file(input_file_path, output_dir);
    }

    return 0;
}

void translate_vm_file_to_asm_file(const std::string& path, const std::string& output_dir) {
    std::string basename = get_basename(path);
    std::string output_file_path = output_dir + basename + ".asm";

    AsmMapper code_mapper(output_file_path, basename);
    VMParser parser(path);

    // code_mapper.write_bootstrap_init();

    while (parser.has_more_lines()) {
        parser.advance();
        switch (parser.instruction_type()) {
            case VMOperationType::C_ARITHMETIC:
                code_mapper.write_arithmetic(parser.get_curr_instruction());
                break;
            case VMOperationType::C_PUSH:
                code_mapper.write_push(parser.get_curr_instruction(), parser.arg1(), parser.arg2());
                break;
            case VMOperationType::C_POP:
                code_mapper.write_pop(parser.get_curr_instruction(), parser.arg1(), parser.arg2());
                break;
            case VMOperationType::C_LABEL:
                code_mapper.write_label(parser.get_curr_instruction(), parser.arg1(), parser.curr_function_name());
                break;
            case VMOperationType::C_GOTO:
                code_mapper.write_goto(parser.get_curr_instruction(), parser.arg1(), parser.curr_function_name());
                break;
            case VMOperationType::C_IF:
                code_mapper.write_if(parser.get_curr_instruction(), parser.arg1(), parser.curr_function_name());
                break;
            case VMOperationType::C_FUNCTION:
                code_mapper.write_function(parser.get_curr_instruction(), parser.arg1(), parser.arg2());
                break;
            case VMOperationType::C_CALL:
                code_mapper.write_call(parser.get_curr_instruction(), parser.arg1(), parser.arg2(), parser.get_return_couter());
                break;
            case VMOperationType::C_RETURN:
                code_mapper.write_return(parser.get_curr_instruction(), parser.curr_function_name());
                break;
            default:
                break;
        }
    }
    code_mapper.write_inf_loop();
    code_mapper.close();
}

std::string get_directory_of_file(const std::string& path) {
    if (path.empty() || path == "/") return "/";

    // Get the index of the last instance of '/'. That will be the stopping point.
    int last_slash_index = path.find_last_of("/");
    if (last_slash_index == std::string::npos) throw new std::invalid_argument("Invalid path supplied: " + path);

    return path.substr(0, last_slash_index + 1);
}

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
    if (!std::regex_search(filename, matches, basename_pattern)) {
        return filename;
    } else {
        return matches[1];
    }
}

bool is_vm_file(const std::string& path) {
    int back = path.size() - 1;
    return path[back - 2] == '.' && path[back - 1] == 'v' &&  path[back] == 'm';
}

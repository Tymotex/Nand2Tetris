#include "utils/Colouriser.h"
#include "utils/JackSourceFileUtilities.h"
#include "LexicalAnalyser.h"
#include "CompilationEngine.h"
#include <iostream>
#include <filesystem>
#include <regex>

// Given the path to a Jack source file, attempts to compile it to the
// intermediate VM language and emit a corresponding .vm file.
void translate_jack_file_to_vm(const std::string& path);

// Prints verbose lexical analyser output.
void show_tokeniser_debug_info(std::shared_ptr<LexicalAnalyser> lexical_analyser);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Insufficient arguments. Please supply a path "
                  << "to the .jack source file or directory.\n";
        exit(1);
    } else if (argc > 2) {
        std::cerr << "Too many arguments. Please supply exactly 1 argument.\n";
        exit(1);
    }

    std::string input_file_path = argv[1];
    std::string basename = get_basename(input_file_path);
    std::string output_dir = std::filesystem::is_directory(input_file_path) ?
        get_basename(input_file_path) : 
        get_directory_of_file(input_file_path);

    // If the given path points to a directory, then process all .jack files
    // found in that directory. Otherwise, process a single file.
    if (std::filesystem::is_directory(input_file_path)) {
        std::cout << argv[0]
                  << Colour::GREEN
                  << ": Translating all .jack files in the given directory.\n"
                  << Colour::RESET;

        // Translate each .jack file in the given directory.
        for (std::filesystem::directory_entry each_file : std::filesystem::directory_iterator(input_file_path)) {
            if (is_jack_file(each_file.path())) {
                std::string path = each_file.path();
                std::cout << argv[0]
                          << Colour::BLUE
                          << ": Processing "
                          << path 
                          << Colour::RESET
                          << "\n";
                translate_jack_file_to_vm(path);
            }
        }
    } else {
        // Translate a single .jack file.
        std::cout << argv[0]
                  << Colour::GREEN
                  << ": Translating a single file.\n\n"
                  << Colour::RESET;
        translate_jack_file_to_vm(input_file_path);
    }
    return 0;
}

void translate_jack_file_to_vm(const std::string& jack_path) {
    // Compilation artifact paths.
    std::string token_xml_output_path =
        get_directory_of_file(jack_path) + get_basename(jack_path) + "T.xml";
    std::string vm_file_path =
        get_directory_of_file(jack_path) + get_basename(jack_path) + ".vm";
    std::string xml_file_path =
        get_directory_of_file(jack_path) + get_basename(jack_path) + ".xml";

    // Initialising the lexical analyser.
    std::ifstream in_jack_stream(jack_path);
    std::ofstream out_token_xml_stream(token_xml_output_path);
    std::shared_ptr<LexicalAnalyser> lexical_analyser =
        std::make_shared<LexicalAnalyser>(in_jack_stream, out_token_xml_stream);

    // Initialising the compilation engine.
    std::ofstream out_vm_stream(vm_file_path);
    std::ofstream out_xml_stream(xml_file_path);
    CompilationEngine compilation_engine(lexical_analyser, out_vm_stream, out_xml_stream);
    
    // Advance the token stream until the `class` token is reached.
    if (!lexical_analyser->try_advance_until_class_declaration())
        throw JackSyntaxError(*lexical_analyser, "Could not find class declaration.");

    // Kick off the recursive descent parsing process.
    compilation_engine.compile_class();

    std::cout << Colour::MAGENTA
              << "\tToken XML file path:  "
              << token_xml_output_path
              << Colour::RESET
              << "\n";
    std::cout << Colour::MAGENTA
              << "\tOutput XML file path: "
              << xml_file_path
              << Colour::RESET
              << "\n";
    std::cout << Colour::MAGENTA
              << "\tVM file path: "
              << vm_file_path
              << Colour::RESET
              << "\n";
}



#include "utils/Colouriser.h"
#include "utils/JackSourceFileUtilities.h"
#include "LexicalAnalyser.h"
#include "Parser.h"
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
        std::cout << argv[0]
                  << Colour::GREEN
                  << ": Translating a single file.\n\n"
                  << Colour::RESET;
        translate_jack_file_to_vm(input_file_path);
    }
    return 0;
}

void translate_jack_file_to_vm(const std::string& path) {
    std::string output_file_path =
        get_directory_of_file(path) + "/" + get_basename(path) + ".xml";
    std::string token_xml_output_path =
        get_directory_of_file(path) + "/" + get_basename(path) + "T.xml";

    std::shared_ptr<LexicalAnalyser> lexical_analyser =
        std::make_shared<LexicalAnalyser>(path, token_xml_output_path);
    std::ofstream output_stream(output_file_path);
    Parser parser(lexical_analyser, output_stream);
    
    while (lexical_analyser->try_advance()) {
        // show_tokeniser_debug_info(lexical_analyser);
    }
    lexical_analyser->reset();

    while (lexical_analyser->try_advance()) {
        show_tokeniser_debug_info(lexical_analyser);
    }
    // Advance the token stream until the `class` token is reached.
    // while (lexical_analyser->try_advance() &&
    //        lexical_analyser->token_type() != TokenType::KEYWORD &&
    //        lexical_analyser->keyword() != Keyword::CLASS) {
    // }

    // Kick off the recursive descent parsing process.
    // parser.compile_class();
}

void show_tokeniser_debug_info(std::shared_ptr<LexicalAnalyser> lexical_analyser) {
    std::cout << Colour::BLUE;
    switch (lexical_analyser->token_type()) {
        case TokenType::KEYWORD:
            std::cout << "\tKeyword:    " << lexical_analyser->get_str_value() << "\n";
            break;
        case TokenType::IDENTIFIER:
            std::cout << "\tIdentifier: " << lexical_analyser->identifier() << "\n";
            break;
        case TokenType::SYMBOL:
            std::cout << "\tSymbol:     " << lexical_analyser->symbol() << "\n";
            break;
        case TokenType::INT_CONST:
            std::cout << "\tInt const:  " << lexical_analyser->get_int_value() << "\n";
            break;
        case TokenType::STRING_CONST:
            std::cout << "\tStr const:  " << lexical_analyser->get_str_value() << "\n";
            break;
        case TokenType::COMMENT:
            std::cout << "\tComment:    " << lexical_analyser->get_str_value() << "\n";
            break;
        default:
            break;
    }
    std::cout << Colour::RESET;
}

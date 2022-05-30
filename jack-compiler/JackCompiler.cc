// For each terminal element Element, output <Element>

#include "LexicalAnalyser.h"
#include "Parser.h"
#include <iostream>
#include <filesystem>
#include <regex>

// Extracts the basename from a given path.
// Eg. Given "/home/linus/hello.txt", `get_basename` returns "hello".
std::string get_basename(std::string path);

// Determines whether the given file has the .jack file extension.
bool is_jack_file(const std::string& path);

// Returns the same path that was given, but one level back.
// Eg. Given "/home/linus/hello.txt", `get_directory_of_file` returns 
//     "/home/linus".
std::string get_directory_of_file(const std::string& path);

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
                  << ": Translating all .jack files in the given directory.\n";

        // Translate each .jack file in the given directory.
        for (std::filesystem::directory_entry each_file : std::filesystem::directory_iterator(input_file_path)) {
            if (is_jack_file(each_file.path())) {
                std::cout << argv[0] << ": Processing " << each_file.path() << "\n";
                std::string path = each_file.path();
                // TODO: begin translation
            }
        }
    } else {
        std::cout << argv[0] << ": Translating a single file.\n\n";
        // TODO: begin translation
    }

    std::cout << "Output path: " << output_dir << std::endl;
    return 0;
}

std::string get_directory_of_file(const std::string& path) {
    if (path.empty() || path == "/") return "/";

    // Get the index of the last instance of '/'. That will be the stopping point.
    int last_slash_index = path.find_last_of("/");
    if (last_slash_index == std::string::npos) {
        std::cerr << "Invalid path supplied: " + path;
        exit(1);
    }

    return path.substr(0, last_slash_index + 1);
}

std::string get_basename(std::string path) {
    // Strip trailing '/' otherwise the basename will be empty.
    if (path.back() == '/') path.pop_back();

    // Locate the last instance of / in the given path, if it exists and use
    // that as the starting index to extract the basename.
    int start_index = path.find_last_of("/");
    if (start_index == std::string::npos) {
        start_index = -1;
    }
    std::string filename = path.substr(start_index + 1);
    std::regex basename_pattern(R"(^(.*)\.jack$)");
    std::smatch matches;
    if (!std::regex_search(filename, matches, basename_pattern)) {
        return filename;
    } else {
        return matches[1];
    }
}

bool is_jack_file(const std::string& path) {
    int back = path.size() - 1;
    return path[back - 4] == '.' &&
        path[back - 3] == 'j' &&
        path[back - 2] == 'a' &&
        path[back - 1] == 'c' &&
        path[back]     == 'k';
}

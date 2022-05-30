#include "JackSourceFileUtilities.h"
#include <iostream>
#include <string>
#include <regex>

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

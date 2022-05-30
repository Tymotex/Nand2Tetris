#ifndef JACK_COMPILER_UTILS_H
#define JACK_COMPILER_UTILS_H

#include <string>

// Extracts the basename from a given path.
// Eg. Given "/home/linus/hello.txt", `get_basename` returns "hello".
std::string get_basename(std::string path);

// Determines whether the given file has the .jack file extension.
bool is_jack_file(const std::string& path);

// Returns the same path that was given, but one level back.
// Eg. Given "/home/linus/hello.txt", `get_directory_of_file` returns 
//     "/home/linus".
std::string get_directory_of_file(const std::string& path);

#endif

#include "LexicalAnalyser.h"
#include <string>

LexicalAnalyser::LexicalAnalyser(const std::string& source_jack_file_path) {
}


JackSyntaxError::JackSyntaxError(char const* const message) throw()
    : _message(message) {
}

char const* JackSyntaxError::what() const throw() {
    return _message;
}


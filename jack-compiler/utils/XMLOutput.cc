#include "XMLOutput.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <unordered_map>

std::unordered_map<char, std::string> XMLOutput::char_escape_dict = {
    {'<', "lt"},
    {'>', "gt"},
    {'"', "quot"},
    {'&', "amp"}
};

XMLOutput::XMLOutput(const std::string& output_path, const bool& should_indent,
        const bool& should_log) 
    : _output_file(std::ofstream(output_path)),
      _indent_level(0),
      _open_elements(std::stack<std::string>()),
      _should_indent(should_indent),
      _should_log(should_log),
      _indent_size(2) {
}

XMLOutput::~XMLOutput() {
    close();
}

void XMLOutput::form_xml(const std::string& element, const std::string& inner_xml) {
    indent();
    _output_file << "<" + element + "> " + escape_xml_str(inner_xml) + " </" + element + ">\n";
    if (_should_log)
        std::cout << "<" + element + "> " + escape_xml_str(inner_xml) + " </" + element + ">\n";
}

void XMLOutput::open_xml(const std::string& element) {
    indent();
    _output_file << "<" + element + ">\n";
    if (_should_log)
        std::cout << "<" + element + ">\n";
    _open_elements.push(element);
    ++_indent_level;
}

void XMLOutput::close_xml() {
    std::string closing_element = _open_elements.top();
    _open_elements.pop();
    --_indent_level;
    indent();
    _output_file << "</" + closing_element + ">\n";
    if (_should_log)
        std::cout << "</" + closing_element + ">\n";
}

void XMLOutput::close() {
    _output_file.close();
}

void XMLOutput::indent() {
    if (!_should_indent) return;
    for (int i = 0; i < _indent_level; ++i) {
        for (int j = 0; j < _indent_size; ++j) {
            _output_file << " ";
            if (_should_indent)
                std::cout << " ";
        }
    }
}

std::string XMLOutput::escape_xml_str(const std::string& s) {
    std::string escaped = "";

    // Map all invalid characters to their escaped equivalent.
    // Eg. maps '<' to '&lt;'.
    for (const char& c : s) {
        if (char_escape_dict.find(c) != char_escape_dict.end()) {
            escaped += "&" + char_escape_dict[c] + ";";
        } else {
            escaped.push_back(c);
        }
    }

    return escaped;
}

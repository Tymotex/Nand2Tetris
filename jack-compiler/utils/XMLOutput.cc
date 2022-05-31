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

XMLOutput::XMLOutput(const std::string& output_path, const bool& should_indent) 
    : _output_file(std::ofstream(output_path)),
      _indent_level(0),
      _open_elements(std::stack<std::string>()),
      _should_indent(should_indent) {
}

XMLOutput::~XMLOutput() {
    close();
}

void XMLOutput::form_xml(const std::string& element, const std::string& inner_xml) {
    indent();
    _output_file << "<" + element + "> " + escape_xml_str(inner_xml) + " </" + element + ">\n";
}

void XMLOutput::open_xml(const std::string& element) {
    indent();
    _output_file << "<" + element + ">\n";
    _open_elements.push(element);
    ++_indent_level;
}

void XMLOutput::close_xml() {
    std::string closing_element = _open_elements.top();
    _open_elements.pop();
    --_indent_level;
    indent();
    _output_file << "</" + closing_element + ">\n";
}

void XMLOutput::close() {
    _output_file.close();
}

void XMLOutput::indent() {
    if (_should_indent)
        for (int i = 0; i < _indent_level; ++i)
            _output_file << "\t";
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

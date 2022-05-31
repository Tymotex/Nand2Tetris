#include "XMLUtilities.h"
#include <algorithm>
#include <string>
#include <unordered_map>

std::unordered_map<char, std::string> XMLUtilities::char_escape_dict = {
    {'<', "lt"},
    {'>', "gt"},
    {'"', "quot"},
    {'&', "amp"}
};

std::string XMLUtilities::form_xml(const std::string& element,
        const std::string& inner_xml, const bool& inline_format) {
    return "<" + element + "> " + XMLUtilities::escape_xml_str(inner_xml) + " </" + element + ">\n";
}

std::string XMLUtilities::escape_xml_str(const std::string& s) {
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

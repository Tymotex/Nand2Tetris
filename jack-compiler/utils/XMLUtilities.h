#ifndef XML_UTILITIES_H
#define XML_UTILITIES_H

#include <string>
#include <unordered_map>

class XMLUtilities {
public:
    // Stores the mappings between invalid XML characters and their escape
    // code. Eg. it holds mappings like '<' to "lt" and '"' to 'quot'.
    static std::unordered_map<char, std::string> char_escape_dict;

    /**
     * Produces a raw formatted string containing the XML code for a single
     * element and its children.
     */
    static std::string form_xml(const std::string& element,
        const std::string& inner_xml, const bool& inline_format);

    /**
     * Escapes all illegal XML characters in the given string.
     * Eg. `escape_str("I <3 you") == "I &lt;3 you"`
     */
    static std::string escape_xml_str(const std::string& s);
};

#endif

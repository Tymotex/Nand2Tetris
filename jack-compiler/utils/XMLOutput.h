#ifndef XML_UTILITIES_H
#define XML_UTILITIES_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <stack>

class XMLOutput {
public:
    // Stores the mappings between invalid XML characters and their escape
    // code. Eg. it holds mappings like '<' to "lt" and '"' to 'quot'.
    static std::unordered_map<char, std::string> char_escape_dict;

    /**
     * Opens a new XML file for writing.
     */
    explicit XMLOutput(const std::string& output_path, const bool& should_indent);

    ~XMLOutput();

    /**
     * Produces a raw formatted string containing the XML code for a single
     * element and its children.
     */
    void form_xml(const std::string& element, const std::string& inner_xml);

    /**
     * Opens a new XML element. You must also call `close` when you are done
     * writing children elements to this open XML element.
     */
    void open_xml(const std::string& element);

    /**
     * Closes off the currently open XML tag. It is the caller's responsibility
     * to ensure that every open XML tag has a corresponding closing tag.
     */
    void close_xml();

    /**
     * Escapes all illegal XML characters in the given string.
     * Eg. `escape_str("I <3 you") == "I &lt;3 you"`
     */
    std::string escape_xml_str(const std::string& s);

    /**
     * Closes off the output stream.
     */
    void close();

private:
    // XML output file.
    std::ofstream _output_file;

    // Book-keeping.
    int _indent_level;
    std::stack<std::string> _open_elements;

    // Options.
    bool _should_indent;

    /**
     * Writes `_indent_level` tab characters.
     */
    void indent();
};

#endif

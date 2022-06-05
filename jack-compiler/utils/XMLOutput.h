#ifndef XML_UTILITIES_H
#define XML_UTILITIES_H

#include <fstream>
#include <string>
#include <unordered_map>
#include <stack>

class XMLOutputNode;

class XMLOutput {
public:
    // Stores the mappings between invalid XML characters and their escape
    // code. Eg. it holds mappings like '<' to "lt" and '"' to 'quot'.
    static std::unordered_map<char, std::string> char_escape_dict;

    /**
     * Opens a new XML file for writing.
     */
    explicit XMLOutput(std::ostream& output_stream,
        const bool& should_indent, const bool& should_log);

    ~XMLOutput();

    /**
     * Produces a raw formatted string containing the XML code for a single
     * element and its children.
     */
    void form_xml(const std::string& element, const std::string& inner_xml);

    /**
     * Opens a new XML element. If `inner_xml` content is provided, then the XML
     * element is immediately closed off, otherwise it will be closed when the
     * XMLOutputNode is destroyed.
     */
    XMLOutputNode open_xml(const std::string& element);
    void open_xml(const std::string& element, const std::string& inner_xml);

    /**
     * Closes off the currently open XML tag. It is the caller's responsibility
     * to ensure that every open XML tag has a corresponding closing tag.
     */
    void close_xml();

    /**
     * Closes off the output stream.
     */
    void close();

private:
    // XML output file.
    std::ostream& _output_stream;

    // Book-keeping.
    int _indent_level;
    std::stack<std::string> _open_elements;

    // Options.
    bool _should_indent;
    bool _should_log;
    int _indent_size;

    /**
     * Writes `_indent_level` tab characters.
     */
    void indent();
    
    /**
     * Escapes all illegal XML characters in the given string.
     * Eg. `escape_str("I <3 you") == "I &lt;3 you"`
     */
    std::string escape_xml_str(const std::string& s);
};

class XMLOutputNode {
public:
    // Closes off the currently open XML tag.
    ~XMLOutputNode();
    void add_child(const std::string& child);
private:
    XMLOutput& manager;
};

#endif

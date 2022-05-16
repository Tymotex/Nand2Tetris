#ifndef VMPARSER_H
#define VMPARSER_H

#include <string>
#include <fstream>

enum class VMOperationType {
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_RETURN,
    C_CALL,
    INVALID
};

class VMParser {
public:
    explicit VMParser(const std::string& vm_source_file_path);

    /**
     * Determines if there exists more lines of the input .vm file to parse.
     */
    bool has_more_lines();

    /**
     * Advances to the next VM instruction, skipping past any non-instructions
     * like comments.
     * Assumes more lines are present in the .vm file.
     */
    void advance();

    /**
     * Gets the current line of VM's command type. It can be an arithmetic type,
     * push/pop, label, goto, etc. 
     */
    VMOperationType command_type();

    /**
     * Extracts the first argument from the current VM instruction. In the case
     * of a push/pop operation, this would obtain the segment argument.
     */
    std::string arg1();
    
    /**
     * Extracts the second argument from the current VM instruction. In the case
     * of a push/pop operation, this would obtain the index value.
     */
    int arg2();

    /**
     * Retrieves the current instruction.
     */
    std::string get_curr_instruction();

private:
    std::ifstream _vm_in;
    std::string _curr_instruction;
    int _curr_line;
    VMOperationType _command_type;

    std::string _arg1;
    int _arg2;

    // Applies transformations to the current instruction to normalise it for
    // parsing.
    void preprocess();

    // Parses the current instruction and populates _command_type, _arg1 and
    // _arg2. Returns true if the instruction is valid.
    bool parse();

    void show_instruction_debug_info();
};

#endif

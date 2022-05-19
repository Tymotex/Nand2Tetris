#ifndef VMPARSER_H
#define VMPARSER_H

#include <string>
#include <fstream>
#include <unordered_set>
#include <regex>

enum class VMOperationType {
    C_ARITHMETIC,
    C_PUSH,
    C_POP,
    C_LABEL,
    C_GOTO,
    C_IF,
    C_FUNCTION,
    C_CALL,
    C_RETURN,
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
    VMOperationType instruction_type();

    /**
     * Retrieves the name of the function whose definition the cursor, 
     * `_curr_line`, is within.
     */
    std::string curr_function_name();

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
    // Set of arithmetic-logic VM instructions:
    static std::unordered_set<std::string> _arithmetic_logic_operators;

    // Stream of source VM instructions to map to Hack assembly. 
    std::ifstream _vm_in;

    // Cursor variables. These will always progress forward.
    std::string _curr_instruction;
    std::string _curr_function_name;
    int _curr_line;

    // The type of the command for the current instruction. The `VMTranslator`
    // class uses this to know how to delegate translation tasks to `AsmMapper`.
    VMOperationType _instruction_type;

    // Some VM instructions will have arguments. Eg. `push <segment> <val>`.
    // There will never be more than 2 arguments, and the second will always
    // be an integer no matter what the instruction type is.
    std::string _arg1;
    int _arg2;

    // Regex patterns for extracting arguments (via capture groups) from various
    // VM instruction formats.
    // TODO:
    
    // Applies transformations to the current instruction to normalise it for
    // parsing.
    void preprocess();

    // Parses the current instruction and populates _command_type, _arg1 and
    // _arg2. Returns true if the instruction is valid.
    bool parse();

    // Prints to `stdout` parsing debugging information.
    void show_instruction_debug_info();
};

#endif

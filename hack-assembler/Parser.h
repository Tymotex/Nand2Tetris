#include <string>
#include <vector>

/**
 * Hack assembly instructions are either A-instructions or C-instructions.
 * Labels are not really instructions, they're just symbolic markers, but here
 * we treat them as another instruction type.
 */
enum class InstructionType {
    A_INSTRUCTION,
    C_INSTRUCTION,
    L_INSTRUCTION
};

/**
 * Each Parser instance is responsible for opening a .asm file, reading each
 * line, extracting out the micro-codes constituting the 16-bit instruction and
 * maintaining state about where it is up to in parsing the file.
 */
class Parser {
public:
    /**
     * Opens the given .asm file and loads its contents into a Parser object.
     */
    explicit Parser(std::string asm_source_file_path);

    /**
     * Determines if there are more lines to be parsed or whether the end of the
     * file has been reached.
     */
    bool has_more_lines();

    /**
     * Moves the parser's cursor to the next instruction and makes that the
     * current instruction. This moves the cursor past any whitespaces and
     * comments that exist between the current instruction and the next
     * instruction. 
     */
    bool advance();

    /**
     * Returns the type of the current instruction.
     */
    InstructionType instruction_type();

    /**
     * Extracts out the symbol that's referenced in the current A-instruction or
     * L-instruction.
     * It is invalid to invoke this function on a C-instruction.
     */
    std::string symbol();

    /**
     * Extracts out the destination of the current C-instruction. 
     * It is invalid to invoke this function on anything but a C-instruction.
     */
    std::string dest();

    /**
     * Extracts the comp code from the C-instruction.  
     * It is invalid to invoke this function on anything but a C-instruction.
     */
    std::string comp();

    /**
     * Extracts the jump code from the C-instruction.
     * It is invalid to invoke this function on anything but a C-instruction.
     */
    std::string jump();

private:
    std::vector<std::string> _asm_source;
};

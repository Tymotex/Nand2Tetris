#ifndef ASM_MAPPER_H
#define ASM_MAPPER_H

#include <string>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

class AsmMapper {
public:
    /**
     * Opens a file output stream for writing the translated VM instructions to.
     * Expects the output file path to have the .asm extension.
     */
    explicit AsmMapper(const std::string& asm_output_file_path);

    /**
     * Writes the corresponding Hack assembly code for the given command into
     * the output .asm file.
     */
    void write_arithmetic(const std::string& command);

    /**
     * Writes the corresponding Hack assembly code for the given push command.
     */
    void write_push(const std::string& command, const std::string& segment,
        const int& index);

    /**
     * Writes the corresponding Hack assembly code for the given pop command.
     */
    void write_pop(const std::string& command, const std::string& segment,
        const int& index);

    /**
     * Inserts a final infinite loop at the end of the Hack assembly program.
     */
    void write_inf_loop();

    /**
     * Closes the output file stream.
     */
    void close();
private:
    std::ofstream _asm_out;

    // Pre-defined segments: static and temp.
    // The base addresses of these segments are:
    //      static, 16  (spanning RAM[16..255])
    //      temp, 5     (spanning RAM[5..12])
    static std::unordered_map<std::string, int> _predef_segment_base_addresses;

    // Initialised segments: local, argument, this, that.
    // The base addresses of these segments are stored in:
    //      LCL (RAM[1])
    //      ARG (RAM[2])
    //      THIS (RAM[3])
    //      THAT (RAM[4])
    static std::unordered_map<std::string, std::string> _init_segment_addr_registers;

    // Virtual segments: constant
    static std::string _constant_segment;

    // Pointer segment: pointer
    static std::string _pointer_segment;

    static std::unordered_map<std::string, const char> _arithmetic_logical_binary_op;
    static std::unordered_map<std::string, const char> _arithmetic_logical_unary_op;
    static std::unordered_map<std::string, std::string> _comparison_op;

    // For comparison ops, we need to create a label to jump to and we must not
    // create name collisions.
    int _label_count;
};

#endif

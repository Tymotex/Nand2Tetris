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
     * Opens a new write `ofstream` for a new .vm input file. Closes the current
     * `ofstream`.
     */
    void start_new_translation_unit(const std::string& source_vm_path);

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
     * Writes the corresponding Hack assembly code for declaring a new 
     * non-function label.
     */
    void write_label(const std::string& label);

    /**
     * Writes the corresponding Hack assembly code for unconditionally jumping
     * to a new instruction address.
     */
    void write_goto(const std::string& label);

    /**
     * Writes the corresponding Hack assembly code for conditionally jumping to
     * a new instruction address.
     * When if-goto is used, we only jump if the item at the top of the stack
     * is non-zero.
     */
    void write_if(const std::string& label);

    /**
     * Writes the corresponding Hack assembly code for declaring a new function.
     */
    void write_function(const std::string& function_name, const int& num_params);

    /**
     * Writes the corresponding Hack assembly code for invoking a function.
     */
    void write_call(const std::string& function_name, const int& num_params);

    /**
     * Writes the corresponding Hack assembly code for terminating the current
     * function, restoring the caller's state and resuming execution from where
     * the caller invoked the function.
     */
    void write_return();

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

    // VM instruction to Hack instruction/operator maps.
    static std::unordered_map<std::string, const char> _arithmetic_logical_binary_op;
    static std::unordered_map<std::string, const char> _arithmetic_logical_unary_op;
    static std::unordered_map<std::string, std::string> _comparison_op;

    // For comparison ops, we need to create a label to jump to and we must not
    // create name collisions.
    int _label_count;

    // Writes the assembly code necessary to push a value in D onto the stack.
    void push_to_stack();
    // Writes the assembly code necessary to pop a value from the stack into D.
    void pop_from_stack();
};

#endif

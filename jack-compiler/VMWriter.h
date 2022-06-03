#ifndef VM_WRITER_H
#define VM_WRITER_H

#include <string>
#include <ostream>

enum class VirtualMemorySegment {
    CONSTANT, ARGUMENT, LOCAL, STATIC, THIS, THAT, POINTER, TEMP
};

class VMWriter {
public:
    /**
     * Creates a VMWriter that outputs all translated VM code to the given
     * stream.
     */
    explicit VMWriter(std::ostream& stream);

    /**
     * Closes the output stream.
     */
    ~VMWriter();

    /**
     * Writes to the output stream the VM push operation: `push segment index`.
     */
    void write_push(const VirtualMemorySegment& segment, const int index);

    /**
     * Writes to the output stream the VM pop operation: `pop segment index`.
     */
    void write_pop(const VirtualMemorySegment& segment, const int index);

    /**
     * Writes to the output stream the corresponding VM arithmetic/logical
     * operator. 
     * Eg. given "+", the VMWriter emits the VM instruction 'add' .
     */
    void write_arithmetic(const std::string& operator_symbol);

    /**
     * Writes to the output stream the VM instruction for declaring a label.
     */
    void write_label(const std::string& label);

    /**
     * Writes to the output stream the VM instruction for unconditionally
     * jumping to the given label.
     */
    void write_goto(const std::string& label);

    /**
     * Writes to the output stream the VM instruction for *conditionally*
     * jumping to the given label. Remember, this implicitly compares the value
     * at the top of the VM's stack against 0.
     */
    void write_if(const std::string& label);

    /**
     * Writes to the output stream the VM instruction for invoking a function.
     */
    void write_call(const std::string& function_name, const int num_args);

    /**
     * Writes to the output stream the VM instruction for declaring a function.
     */
    void write_function(const std::string& function_name, const int num_variables);

    /**
     * Writes to the output stream the VM instruction for returning from a
     * function context.
     */
    void write_return();
private:
    std::ostream& _vm_out;
};

#endif

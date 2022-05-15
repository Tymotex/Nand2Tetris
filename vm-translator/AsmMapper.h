#include <string>
#include <fstream>
#include <unordered_map>

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
     * Writes the corresponding Hack assembly code for the given push/pop 
     * command.
     */
    void write_push_pop(const std::string& command, const std::string& segment,
        const int& index);

    /**
     * Closes the output file stream.
     */
    void close();
private:
    std::ofstream _asm_out;
    static std::unordered_map<std::string, int> _segment_base_addresses;
    static std::unordered_map<std::string, std::string> _arithmetic_logical_op;
};

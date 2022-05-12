#include <string>
#include <unordered_map>

/**
 * Maps assembly instructions into 0s and 1s.
 */
class MachineCodeMapper {
public:
    /**
     * Maps the given assembly C-instruction's destination into a string of 0s
     * and 1s.
     */
    std::string dest(std::string asm_code);

    /**
     * Maps the given assembly C-instruction's comp-code into a string of 0s
     * and 1s.
     */
    std::string comp(std::string asm_code);

    /**
     * Maps the given assembly C-instruction's jump code into a string of 0s
     * and 1s.
     */
    std::string jump(std::string asm_code);

    /**
     * Gets the given assembly A-instruction's address as a string of 0s and 1s.
     */
    std::string get_address(std::string asm_code);
private:
    static std::unordered_map<std::string, std::string> dest_to_code;
    static std::unordered_map<std::string, std::string> comp_to_code;
    static std::unordered_map<std::string, std::string> jump_to_code;
};

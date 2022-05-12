#ifndef SYMBOL_TABLE_HACK
#define SYMBOL_TABLE_HACK

#include <string>
#include <unordered_map>

/**
 * Stores an unordered set of symbol name to numeric address key-value pairs.
 * To be used in the first pass of an assembler to note all symbols and the
 * instruction addresses where they should reference.
 */
class SymbolTable {
public:
    explicit SymbolTable();

    /**
     * Inserts a new symbol and its address.
     */
    void add_entry(std::string symbol, int address);

    /**
     * Determines whether the symbol has been encountered in the past and has
     * had an address assigned to it.
     */
    bool contains (std::string symbol);

    /**
     * Fetches the address associated with the given symbol.
     * Assumes that the symbol table contains the given symbol.
     */
    int get_address(std::string symbol);
private:
    std::unordered_map<std::string, int> _symbol_table;
};

#endif

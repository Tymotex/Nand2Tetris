#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "LexicalAnalyser.h"
#include <string>
#include <unordered_map>

enum class DeclarationType {
    VAR, FIELD, STATIC, ARGUMENT
};

// Container for symbol properties.
struct SymbolData {
    std::string data_type;
    DeclarationType declaration_type;
    int index;

    SymbolData(std::string data_type, const DeclarationType declaration_type, const int index);
};

/**
 * Maintains a mapping of identifiers to information about their declaration
 * type, data type and index value (a running integer counter). The index value
 * helps the VMWriter determine the correct offset of a RAM register for
 * different virtual memory segments such as `local`, `argument`, `static`.
 */
class SymbolTable {
public:
    /**
     * Constructs a new symbol table with 0 entries.
     */ 
    explicit SymbolTable();

    /**
     * Wipes all entries from the symbol table and resets the 4 indexes to 0,
     * ie. the running integer counts for declaration types: static, field,
     * argument, local.
     * This method is meant to be invoked when a new subroutine is declared.
     */
    void reset();

    /**
     * Inserts a new symbol entry with the given name, data type and variable
     * declaration type (which can only be one of: static, field, argument, 
     * local variable). Assigns the symbol an index value (sourced from 4
     * running counters) behind the scenes.
     */
    void define(const std::string& name, const std::string& data_type,
        DeclarationType declaration_type);

    /**
     * Returns the number of variables with the given declaration type. You'd
     * use this to determine how much memory to allocate to the instantiation of
     * class, for example.
     */
    int var_count(DeclarationType declaration_type);

    /**
     * Accesses the declaration type of the given identifier.
     */
    DeclarationType declaration_type(const std::string& name);

    /**
     * Accesses the data type of the given identifier.
     */
    std::string data_type(const std::string& name);

    /**
     * Accesses the index value assigned to the given identifier.
     */
    int segment_index(const std::string& name);
    
    /**
     * Determines whether the given symbol name exists in the table.
     */
    bool exists(const std::string& name);
private:
    std::unordered_map<DeclarationType, std::unordered_map<std::string, SymbolData>> _symbol_table;
};

#endif

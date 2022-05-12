#include "SymbolTable.h"
#include <unordered_map>
#include <string>

SymbolTable::SymbolTable()
    : _symbol_table(std::unordered_map<std::string, int>()) {}

void SymbolTable::add_entry(std::string symbol, int address) {
    _symbol_table.insert({ symbol, address });
}

bool SymbolTable::contains(std::string symbol) {
    return _symbol_table.find(symbol) != _symbol_table.end();
}

int SymbolTable::get_address(std::string symbol) {
    return _symbol_table[symbol];
}

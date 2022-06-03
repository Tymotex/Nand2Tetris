#include "SymbolTable.h"

SymbolTable::SymbolTable() {
}

void SymbolTable::reset() {

}

void SymbolTable::define(const std::string& name, const std::string& type,
        Keyword declaration_type) {
    
}

int SymbolTable::var_count(Keyword kind) {
    return -1;
}

Keyword SymbolTable::kind_of(const std::string& name) {
    return Keyword::BOOLEAN;
}

std::string SymbolTable::type_of(const std::string& name) {
    return "";
}

int SymbolTable::index_of(const std::string& name) {
    return -1;
}

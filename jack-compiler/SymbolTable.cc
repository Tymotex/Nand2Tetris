#include "SymbolTable.h"
#include "CompilationEngine.h"
#include "LexicalAnalyser.h"

SymbolData::SymbolData(std::string data_type, const DeclarationType declaration_type, const int index)
        : data_type(data_type),
          declaration_type(declaration_type),
          index(index) {
}

SymbolTable::SymbolTable() {
    // Initialise symbol table with standard declaration type identifier maps.
    reset();
}

void SymbolTable::reset() {
    _symbol_table = {
        {DeclarationType::VAR, {}},
        {DeclarationType::FIELD, {}},
        {DeclarationType::STATIC, {}},
        {DeclarationType::ARGUMENT, {}}
    };
}

void SymbolTable::define(const std::string& name, const std::string& data_type,
        DeclarationType declaration_type) {
    if (exists(name))
        throw JackCompilationEngineError("Redeclaration of identifier '" + name + "'.");
    int num_vars = var_count(declaration_type);
    _symbol_table[declaration_type].insert({
        name,
        SymbolData(data_type, declaration_type, num_vars)
    });
}

int SymbolTable::var_count(DeclarationType declaration_type) {
    if (_symbol_table.find(declaration_type) == _symbol_table.end()) 
        throw JackCompilationEngineError("Unknown declaration type");
    return _symbol_table[declaration_type].size();
}

DeclarationType SymbolTable::declaration_type(const std::string& name) {
    for (const auto& it : _symbol_table) {
        const std::unordered_map<std::string, SymbolData>& _identifiers = it.second;
        if (_identifiers.find(name) != _identifiers.end())
            return it.first; 
    }
    throw JackCompilationEngineError("Cannot retrieve declaration type of undefined symbol.");
}

std::string SymbolTable::data_type(const std::string& name) {
    for (const auto& it : _symbol_table) {
        const std::unordered_map<std::string, SymbolData>& _identifiers = it.second;
        if (_identifiers.find(name) != _identifiers.end())
            return _identifiers.at(name).data_type;
    }
    throw JackCompilationEngineError("Cannot retrieve data type of undefined symbol.");
}

int SymbolTable::segment_index(const std::string& name) {
    for (const auto& it : _symbol_table) {
        const std::unordered_map<std::string, SymbolData>& _identifiers = it.second;
        if (_identifiers.find(name) != _identifiers.end())
            return _identifiers.at(name).index;
    }
    throw JackCompilationEngineError("Cannot retrieve segment index of undefined symbol.");
}

bool SymbolTable::exists(const std::string& name) {
    for (const auto& it : _symbol_table) {
        const std::unordered_map<std::string, SymbolData>& _identifiers = it.second;
        if (_identifiers.find(name) != _identifiers.end())
            return true;
    }
    return false;
}

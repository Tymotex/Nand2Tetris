#include "SymbolTable.h"
#include "CompilationEngine.h"
#include "LexicalAnalyser.h"
#include "utils/Colouriser.h"
#include <iostream>

SymbolData::SymbolData(std::string data_type, const DeclarationType declaration_type, const int index)
        : data_type(data_type),
          declaration_type(declaration_type),
          index(index) {
}

std::unordered_map<DeclarationType, std::string> SymbolTable::decl_type_to_str {
    {DeclarationType::VAR, "var"},
    {DeclarationType::FIELD, "field"},
    {DeclarationType::STATIC, "static"},
    {DeclarationType::ARGUMENT, "argument"},
};

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
        const std::string& decl_type_str) {
    if (exists(name))
        throw JackCompilationEngineError("Redeclaration of identifier '" + name + "'.");
    if (!is_valid_data_type(data_type))
        throw JackCompilationEngineError("Invalid data type '" + data_type + "'");

    DeclarationType declaration_type = str_to_declaration_type(decl_type_str);
    int num_vars = var_count(declaration_type);

    _symbol_table[declaration_type].insert({
        name,
        SymbolData(data_type, declaration_type, num_vars)
    });
    std::cout << Colour::GREEN
              << "Defined symbol: ("
              << name << ", " << data_type << ", " << str_declaration_type(declaration_type)
              << ")\n"
              << Colour::RESET;
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

std::string SymbolTable::str_declaration_type(DeclarationType decl_type) {
    if (decl_type_to_str.find(decl_type) == decl_type_to_str.end())
        throw JackCompilationEngineError("Unknown declaration type.");
    return decl_type_to_str[decl_type];
}

bool SymbolTable::is_valid_data_type(const std::string& data_type) {
    return (data_type == "int" ||
        data_type == "char" ||
        data_type == "boolean" ||
        LexicalAnalyser::is_valid_identifier(data_type));
}

DeclarationType SymbolTable::str_to_declaration_type(const std::string& decl_type) {
    // Inefficient linear scan. A performant alternative would be to use a
    // bidirectional map.
    auto it = std::find_if(decl_type_to_str.begin(), decl_type_to_str.end(),
        [decl_type](const auto& it) { return it.second == decl_type; });
    
    if (it == decl_type_to_str.end())
        throw JackCompilationEngineError("Invalid declaration type: '" + decl_type + "'");
    
    return it->first;
}

#include "SymbolTable.h"
#include "CompilationEngine.h"
#include <gtest/gtest.h>

class SymbolTableTestFixture : public ::testing::Test {
protected:    
    SymbolTable symbol_table;

    SymbolTableTestFixture() {}
};

TEST_F(SymbolTableTestFixture, InsertAndRetrieveTest) {
    symbol_table.define("myVar", "int", "var");
    EXPECT_EQ(symbol_table.data_type("myVar"), "int");
    EXPECT_EQ(symbol_table.declaration_type("myVar"), DeclarationType::VAR);
    EXPECT_EQ(symbol_table.segment_index("myVar"), 0);
}

// Verifies that the symbol table manages independent running counters for each
// declaration type.
TEST_F(SymbolTableTestFixture, DeclarationTypeRunningIndexTest){
    symbol_table.define("field1", "int", "field");
    symbol_table.define("static1", "int", "static");
    symbol_table.define("static2", "char", "static");
    symbol_table.define("var1", "int", "var");
    symbol_table.define("var2", "char", "var");
    symbol_table.define("var3", "boolean", "var");

    EXPECT_EQ(symbol_table.segment_index("field1"), 0);
    EXPECT_EQ(symbol_table.segment_index("static1"), 0);
    EXPECT_EQ(symbol_table.segment_index("static2"), 1);
    EXPECT_EQ(symbol_table.segment_index("var1"), 0);
    EXPECT_EQ(symbol_table.segment_index("var2"), 1);
    EXPECT_EQ(symbol_table.segment_index("var3"), 2);
}

// Inserts symbols belonging to different memory segments and checks the correct
// properties about them are stored and retrieved.
TEST_F(SymbolTableTestFixture, MultipleSegmentInsertAndRetrieveTest) {
    symbol_table.define("field1", "int", "field");
    symbol_table.define("static1", "int", "static");
    symbol_table.define("static2", "char", "static");
    symbol_table.define("var1", "int", "var");
    symbol_table.define("var2", "char", "var");
    symbol_table.define("var3", "boolean", "var");
    symbol_table.define("arg1", "int", "argument");

    EXPECT_EQ(symbol_table.declaration_type("field1"), DeclarationType::FIELD);
    EXPECT_EQ(symbol_table.data_type("field1"), "int");

    EXPECT_EQ(symbol_table.declaration_type("static1"), DeclarationType::STATIC);
    EXPECT_EQ(symbol_table.data_type("static1"), "int");
    EXPECT_EQ(symbol_table.declaration_type("static2"), DeclarationType::STATIC);
    EXPECT_EQ(symbol_table.data_type("static2"), "char");

    EXPECT_EQ(symbol_table.declaration_type("var1"), DeclarationType::VAR);
    EXPECT_EQ(symbol_table.data_type("var1"), "int");
    EXPECT_EQ(symbol_table.declaration_type("var2"), DeclarationType::VAR);
    EXPECT_EQ(symbol_table.data_type("var2"), "char");
    EXPECT_EQ(symbol_table.declaration_type("var3"), DeclarationType::VAR);
    EXPECT_EQ(symbol_table.data_type("var3"), "boolean");

    EXPECT_EQ(symbol_table.data_type("arg1"), "int");
}

// Tests that the symbol table records the correct set size of symbols under
// different declaration types.
TEST_F(SymbolTableTestFixture, RetrievesCorrectVariableCountForDeclType) {
    symbol_table.define("field1", "int", "field");
    symbol_table.define("static1", "int", "static");
    symbol_table.define("static2", "char", "static");
    symbol_table.define("var1", "int", "var");
    symbol_table.define("var2", "char", "var");
    symbol_table.define("var3", "boolean", "var");

    EXPECT_EQ(symbol_table.var_count(DeclarationType::FIELD), 1);
    EXPECT_EQ(symbol_table.var_count(DeclarationType::STATIC), 2);
    EXPECT_EQ(symbol_table.var_count(DeclarationType::VAR), 3);
}

// Tests that the symbol table can be wiped multiple times and have all state
// correctly reset (such as the running index counters).
TEST_F(SymbolTableTestFixture, ResetTest) {
    symbol_table.define("var1", "int", "var");
    symbol_table.define("var2", "char", "var");

    EXPECT_TRUE(symbol_table.exists("var1"));
    EXPECT_TRUE(symbol_table.exists("var2"));

    EXPECT_EQ(symbol_table.var_count(DeclarationType::VAR), 2);

    symbol_table.reset();

    EXPECT_EQ(symbol_table.var_count(DeclarationType::VAR), 0);
    EXPECT_FALSE(symbol_table.exists("var1"));
    EXPECT_FALSE(symbol_table.exists("var2"));

    // Re-insert variables and verify the index number starts again from 0.
    symbol_table.define("var1", "int", "var");
    symbol_table.define("var2", "char", "var");

    EXPECT_EQ(symbol_table.segment_index("var1"), 0);
    EXPECT_EQ(symbol_table.segment_index("var2"), 1);
}

// Verifies that a redeclaration of a symbol is recognised and disallowed.
TEST_F(SymbolTableTestFixture, RedeclarationIsDisallowed) {
    symbol_table.define("var1", "int", "var");
    EXPECT_THROW({
        symbol_table.define("var1", "char", "var");
    }, JackCompilationEngineError)
        << "Symbol table needs to disallow the redeclaration of identifiers.";
}

// Verifies that attempting to read a property of an undefined symbol results
// in an exception.
TEST_F(SymbolTableTestFixture, InvalidPropertyRetrievalOnUndefinedSymbolTest) {
    EXPECT_THROW({
        symbol_table.declaration_type("undefinedIdentifier");
    }, JackCompilationEngineError);
    EXPECT_THROW({
        symbol_table.segment_index("undefinedIdentifier");
    }, JackCompilationEngineError);
    EXPECT_THROW({
        symbol_table.data_type("undefinedIdentifier");
    }, JackCompilationEngineError);
}

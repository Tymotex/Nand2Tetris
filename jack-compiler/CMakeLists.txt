cmake_minimum_required(VERSION 3.14)
project(JackCompiler)

set(CMAKE_CXX_STANDARD 17)

# Enable GDB debug info.
add_compile_options(-g)

# ===== Fetch GoogleTest =====
include(FetchContent)
FetchContent_Declare(
	  googletest
	  # Specify the commit hash you depend on and update it regularly.
	  URL https://github.com/google/googletest/archive/e2239ee6043f73722e7aa812a459f54a28552929.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings.
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# ===== Executable =====
add_subdirectory(utils)

add_executable(
    JackCompiler
    JackCompiler.cc
    LexicalAnalyser.cc
    CompilationEngine.cc
    SymbolTable.cc
    VMWriter.cc
)

target_link_libraries(
    JackCompiler PUBLIC utils
)
target_include_directories(
    JackCompiler
    PUBLIC "${PROJECT_BINARY_DIR}"
           "${PROJECT_SOURCE_DIR}/MathFunctions"
)

# ===== Enable GoogleTest =====
# Building the GoogleTest test binaries
enable_testing()

# Add new test files here:
add_executable(
    test_binary
    SymbolTable.cc SymbolTableTest.cc
    VMWriter.cc VMWriterTest.cc
)

target_link_libraries(test_binary gtest_main)

include(GoogleTest)
gtest_discover_tests(test_binary)

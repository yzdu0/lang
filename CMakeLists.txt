cmake_minimum_required(VERSION 3.20)

project(lang
    VERSION 0.1.0
    DESCRIPTION "A small programming language"
    LANGUAGES CXX
)

add_executable(lang
    src/compiler/ast.cpp
    src/compiler/parser.cpp
    src/compiler/token.cpp
    src/compiler/tokenizer.cpp
    src/main.cpp
    src/vm/vm.cpp
)

target_include_directories(lang PRIVATE include)
target_compile_features(lang PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(lang PRIVATE /W4 /permissive-)
else()
    target_compile_options(lang PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_target(run-bytecode
    COMMAND $<TARGET_FILE:lang>
    DEPENDS lang
    COMMENT "Running the bytecode in VM::run_program"
    VERBATIM
)

add_custom_target(print-ast
    COMMAND $<TARGET_FILE:lang> --ast "${CMAKE_CURRENT_SOURCE_DIR}/examples/parser.lang"
    DEPENDS lang
    COMMENT "Printing the AST for examples/parser.lang"
    VERBATIM
)

cmake_minimum_required(VERSION 3.20)

project(lang
    VERSION 0.1.0
    DESCRIPTION "A small programming language"
    LANGUAGES CXX
)

add_executable(lang
    src/main.cpp
    src/vm.cpp
)

target_include_directories(lang PRIVATE include)
target_compile_features(lang PRIVATE cxx_std_20)

if(MSVC)
    target_compile_options(lang PRIVATE /W4 /permissive-)
else()
    target_compile_options(lang PRIVATE -Wall -Wextra -Wpedantic)
endif()

add_custom_target(run-bytecode
    COMMAND $<TARGET_FILE:lang> "${CMAKE_CURRENT_SOURCE_DIR}/examples/bytecode.txt"
    DEPENDS lang
    COMMENT "Running examples/bytecode.txt"
    VERBATIM
)

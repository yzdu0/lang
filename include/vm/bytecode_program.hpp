#pragma once

#include "vm/instruction.hpp"

#include <cstddef>
#include <vector>

class BytecodeProgram {
public:
    struct BytecodeFunction {
        std::size_t entry_ip = 0;
        std::size_t arg_count = 0;
        std::size_t local_count = 0;
    };

    std::vector<Instruction> code;
    std::vector<BytecodeFunction> functions;
    std::size_t local_count = 0;
};

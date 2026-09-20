#pragma once

#include "vm/instruction.hpp"

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

class BytecodeProgram {
public:
    struct LocalMapping {
        std::string name;
        std::int32_t index = 0;
    };

    struct BytecodeFunction {
        std::size_t entry_ip = 0;
        std::size_t arg_count = 0;
        std::size_t local_count = 0;
        std::size_t global_index = 0;
        std::size_t parameter_start = 0;
        std::string name;
        std::vector<LocalMapping> local_mappings;
    };

    std::vector<Instruction> code;
    std::vector<BytecodeFunction> functions;
    std::size_t local_count = 0;
    std::vector<LocalMapping> local_mappings;
};

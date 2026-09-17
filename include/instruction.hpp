#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>

enum class OpCode : uint8_t {
    DeclareSymbol, // a = symbol index, b = type (0=integer, 1=array, 2=function)

    PushConst,      // a = constant
    PushRef,        // a = symbol index

    Store,          // a = symbol index
    Load,           // a = symbol index

    Print, 

    Add,
    Multiply,
    LessThan,

    PushNewArray,
    ArrayGet,
    ArrayPushBack,

    Call,           // a = function index
    Return,

    Jump,           // a = target
    JumpIfZero,     // a = target
    JumpIfFalse,    // a = target

    Halt
};

struct Instruction {
    OpCode op;
    int32_t a = 0;
    int32_t b = 0;
};
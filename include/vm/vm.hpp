#pragma once

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string_view>
#include <vector>
#include <map>
#include <variant>
#include <memory>
#include <string>
#include <stdio.h>
#include <iostream>
#include <utility>

#include "vm/bytecode_program.hpp"
#include "vm/instruction.hpp"
#include "vm/value.hpp"
#include "vm/callstack.hpp"


class VM {
public:
    VM() = default;
    explicit VM(BytecodeProgram program) : program(std::move(program)) {}

    void run_program();

private:

    BytecodeProgram program;

    struct ArrayObject {
        std::vector<Value> elements;

        static ArrayObject init(){
            ArrayObject x;
            return x;
        }

        void print(){
            for(Value element : elements){
                element.print();
                std::cout << " ";
            }
        }
    };
    
    struct StringObject {
        std::string value;

        void print(){

        }
    };
    
    using HeapObject = std::variant<ArrayObject, StringObject>;

    std::vector<HeapObject> heap;

    std::vector<Value> work_stack;

    CallStack call_stack;

    std::size_t ip = 0;


    void work_stack_push(Value value) {
        work_stack.push_back(value);
    }

    Value work_stack_pop() {
        if (work_stack.empty()) {
            throw std::runtime_error("VM work_stack underflow");
        }

        Value value = work_stack.back();
        work_stack.pop_back();
        return value;
    }

    Value& work_stack_top() {
        if (work_stack.empty()) {
            throw std::runtime_error("VM work_stack is empty");
        }

        return work_stack.back();
    }

    

    void execute_instruction(const Instruction &cur);

    void e_PushConst(const Instruction &cur);

    void e_PushRef(const Instruction &cur);

    void e_Dup(const Instruction&);

    void e_Pop(const Instruction&);

    void e_Print(const Instruction&);

    void e_PushNewArray(const Instruction &cur);

    void e_Load(const Instruction &cur);

    void e_ArrayPushBack(const Instruction &cur);

    void e_Store(const Instruction &cur);

    void e_ArrayNew(const Instruction &cur);

    void e_ArrayGet(const Instruction &cur);

    void e_ArrayLength(const Instruction&);

    void e_ArrayPush(const Instruction&);

    void e_Add(const Instruction &cur);

    void e_Subtract(const Instruction &cur);

    void e_Multiply(const Instruction &cur);

    void e_Divide(const Instruction &cur);

    void e_EQEQ(const Instruction &cur);

    void e_NEQ(const Instruction &cur);

    void e_LessThan(const Instruction&);

    void e_LessEqual(const Instruction&);

    void e_GreaterThan(const Instruction&);

    void e_GreaterEqual(const Instruction&);

    void e_Call(const Instruction &cur);

    void e_CallIndirect(const Instruction &cur);

    void callFunction(std::size_t function_index);

    void e_Return(const Instruction &cur);

    void e_Jump(const Instruction &cur);

    void e_JumpIfZero(const Instruction &cur);
};

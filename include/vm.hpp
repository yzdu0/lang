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

#include "instruction.hpp"


class VM {
public:
    VM() = default;

    void run_program();

private:
    enum class ValueType {
        Int,
        Bool,
        Object,
        Function,
        Null
    };

    struct Value {
        ValueType type;

        union {
            int64_t integer;
            bool boolean;
            uint32_t objectId;
            uint32_t functionId;
        };

        static Value Int(int64_t value) {
            Value v;
            v.type = ValueType::Int;
            v.integer = value;
            return v;
        }

        static Value Bool(bool value) {
            Value v;
            v.type = ValueType::Bool;
            v.boolean = value;
            return v;
        }

        static Value Object(uint32_t id) {
            Value v;
            v.type = ValueType::Object;
            v.objectId = id;
            return v;
        }

        static Value Function(uint32_t id){
            Value v;
            v.type = ValueType::Function;
            v.functionId = id;
            return v; 
        }

        static Value Null(){
            Value v;
            v.type = ValueType::Null;
            return v;
        }

        void print(){
            if(type == ValueType::Int){
                std::cout << integer;
            }
        }
    };

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
    std::vector<Value> locals;

    std::size_t ip = 0; // instruction pointer

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

    void e_PushNewArray(const Instruction &cur);

    void e_Load(const Instruction &cur);

    void e_ArrayPushBack(const Instruction &cur);

    void e_Store(const Instruction &cur);

    void e_ArrayNew(const Instruction &cur);

    void e_Add(const Instruction &cur);

    void e_Multiply(const Instruction &cur);
};

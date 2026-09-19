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

#include "vm/instruction.hpp"

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
        if(type == ValueType::Bool){
            std::cout << (boolean ? "true" : "false");
        }
        if(type == ValueType::Object){
            std::cout << "<object " << objectId << '>';
        }
        if(type == ValueType::Function){
            std::cout << "<function " << functionId << '>';
        }
        if(type == ValueType::Null){
            std::cout << "null";
        }
    }
};

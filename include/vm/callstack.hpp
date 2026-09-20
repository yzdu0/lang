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

class CallStack {
    public:
        class CallFrame {
            public:
                std::vector<Value> locals;
                std::size_t return_address;

                std::vector<Value>::iterator begin() { return locals.begin(); }
                std::vector<Value>::iterator end() { return locals.end(); }
        };

        std::vector<CallFrame> call_stack;

        CallFrame& scope(){
            return call_stack.back();
        }

        std::vector<Value>& get_locals(){
            return call_stack.back().locals;
        }

        std::vector<Value>& get_global_locals(){
            return call_stack.front().locals;
        }

        void add_call_frame(){
            call_stack.push_back({});
        }

        void remove_call_frame(){
            call_stack.pop_back();
        }
};

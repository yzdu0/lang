#include "vm.hpp"
#include "instruction.hpp"
#include <stdio.h>
#include <iostream>

#define X 0
#define Y 1

void VM::run_program() {
    std::vector<Instruction> code;
    /*
    x <-> 0
    */

    
    /*code.push_back({OpCode::DeclareSymbol, X, 0}); // symbol x (int but unknown to vm)
    code.push_back({OpCode::DeclareSymbol, Y, 1}); // symbol y (array but unknown to vm)*/

    for(int i = 0; i < 100; i ++){
        locals.push_back(Value::Null());
    }

    code.push_back({OpCode::PushConst, 5}); // stack_push 5
    code.push_back({OpCode::Store, X});

    code.push_back({OpCode::PushNewArray}); // Similar to PushConst but a reference to an empty array on the heap.
    code.push_back({OpCode::Store, Y});

    code.push_back({OpCode::Load, Y});
    code.push_back({OpCode::PushConst, 6});
    code.push_back({OpCode::ArrayPushBack});

    code.push_back({OpCode::Load, Y});
    code.push_back({OpCode::PushConst, 7});
    code.push_back({OpCode::ArrayPushBack});

    int i = 0;
    while(i < code.size()){
        Instruction cur = code[i];
        execute_instruction(cur);
        i ++;
    }

    for(int i = 0; i < locals.size(); i ++){
        locals[i].print();
        std::cout << "\n";

        if(locals[i].type == ValueType::Null){
            break;
        }
    }
    std::cout << "HEAP: \n";
    //std::cout << heap.size();
    for(int i = 0; i < heap.size(); i ++){
        std::visit([](auto& obj) {
            obj.print();
        }, heap[i]);
        std::cout << "\n";
    }
}

void VM::execute_instruction(const Instruction &cur){
    switch(cur.op){
        case OpCode::PushConst:
            e_PushConst(cur);
            break;
        case OpCode::PushRef:
            e_PushRef(cur);
            break;
        case OpCode::PushNewArray:
            e_PushNewArray(cur);
            break;
        case OpCode::Load:
            e_Load(cur);
            break;
        case OpCode::ArrayPushBack:
            e_ArrayPushBack(cur);
            break;
        case OpCode::Store:
            e_Store(cur);
            break;
        case OpCode::Print:

            break;
    }
}

/*void VM::e_DeclareSymbol(const Instruction &cur){
    switch(cur.b){
        case 0: // Int
            locals.push_back(Value::Int(0));
            break;
        case 1: // Array

            /*locals.push_back(
                Value::Object(heap.size())
            ); // pointer to heap

            heap.push_back(
                ArrayObject::init()
            );
            break;
    }
}*/

void VM::e_PushConst(const Instruction &cur){
    work_stack.push_back(Value::Int(cur.a));
}

void VM::e_PushRef(const Instruction &cur){
    work_stack.push_back(Value::Object(cur.a));
}

void VM::e_Load(const Instruction &cur){
    // a = local index to load
    work_stack.push_back(locals[cur.a]);
}

void VM::e_ArrayPushBack(const Instruction &cur){
    Value val = work_stack_pop();
    Value array_reference = work_stack_pop();

    HeapObject& obj = heap[array_reference.objectId];

    std::get<ArrayObject>(obj).elements.push_back(val);

}

void VM::e_Store(const Instruction &cur){
    Value item = work_stack_pop();

    //std::cout << ref.objectId << "|----";

    locals[cur.a] = item;
}

void VM::e_PushNewArray(const Instruction &cur){
    uint32_t objectId = static_cast<uint32_t>(heap.size());
    
    heap.push_back(ArrayObject{});
    
    work_stack.push_back(Value::Object(objectId));
}
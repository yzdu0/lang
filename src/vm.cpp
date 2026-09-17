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

    /*for(int i = 0; i < 100; i ++){
        call_stack.push_back(Value::Null());
    }*/
    call_stack.add_call_frame();
    call_stack.get_locals().resize(10);

    FunctionTable.push_back({});
    FunctionTable[0].entry_ip = 4;
    FunctionTable[0].arg_count = 1;
    FunctionTable[0].local_count = 1;

    // x = 5
    code.push_back({OpCode::PushConst, 5}); // stack_push 5  //0

    code.push_back({OpCode::Call, 0}); // function id 0
    code.push_back({OpCode::Store, X});

    code.push_back({OpCode::Halt});             // 3

    // square function
    code.push_back({OpCode::Load, 0}); // 4
    code.push_back({OpCode::Load, 0});
    code.push_back({OpCode::Multiply});
    code.push_back({OpCode::Return});

    code.push_back({OpCode::Load, X});

    //int i = 0;
    ip = 0;
    while(ip < code.size()){
        Instruction cur = code[ip];

        execute_instruction(cur);
        ip ++;
    }

    for(Value local : call_stack.get_locals()){
        local.print();
        std::cout << "\n";

        if(local.type == ValueType::Null){
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
        case OpCode::Add:
            e_Add(cur);
            break;
        case OpCode::Multiply:
            e_Multiply(cur);
            break;
        case OpCode::Call:
            e_Call(cur);
            break;
        case OpCode::Return:
            e_Return(cur);
            break;
        case OpCode::Print:

            break;
        case OpCode::Halt:
            ip = 10000;
            break;
    }
}

/*void VM::e_DeclareSymbol(const Instruction &cur){
    switch(cur.b){
        case 0: // Int
            call_stack.push_back(Value::Int(0));
            break;
        case 1: // Array

            /*call_stack.push_back(
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
    work_stack.push_back(call_stack.get_locals()[cur.a]);
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

    call_stack.get_locals()[cur.a] = item;
}

void VM::e_PushNewArray(const Instruction &cur){
    uint32_t objectId = static_cast<uint32_t>(heap.size());
    
    heap.push_back(ArrayObject{});
    
    work_stack.push_back(Value::Object(objectId));
}

void VM::e_Add(const Instruction &cur){
    Value a1 = work_stack_pop();
    Value a2 = work_stack_pop();

    if(a1.type == ValueType::Int && a2.type == ValueType::Int){
        Value a3 = Value::Int(a1.integer + a2.integer);
        work_stack_push(a3);
        //std::cout << "yes";
    } else {
        //std::cout << "yes";
        std::cerr << "addition (+) operator undefined for given type";
    }
}

void VM::e_Multiply(const Instruction &cur){
    Value a1 = work_stack_pop();
    Value a2 = work_stack_pop();

    if(a1.type == ValueType::Int && a2.type == ValueType::Int){
        Value a3 = Value::Int(a1.integer * a2.integer);
        work_stack_push(a3);
    } else {
        std::cerr << "multiplication (*) operator undefined for given type";
    }
}

void VM::e_Call(const Instruction &cur){
    // cur.a = function ID
    Function& fn = FunctionTable[cur.a];

    call_stack.add_call_frame();
    call_stack.get_locals().resize(fn.local_count);
    call_stack.scope().return_address = ip;

    ip = fn.entry_ip - 1;

    for(std::size_t i = fn.arg_count; i > 0; --i){
        call_stack.get_locals()[i - 1] = work_stack_pop();
    }
}

void VM::e_Return(const Instruction &cur){
    const std::size_t return_address = call_stack.scope().return_address;

    call_stack.remove_call_frame();
    ip = return_address;
}

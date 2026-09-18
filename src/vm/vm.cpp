#include "vm/vm.hpp"
#include "vm/instruction.hpp"
#include <stdio.h>
#include <iostream>

#define X 0
#define Y 1

void VM::run_program() {
    std::vector<Instruction> code;

    call_stack.add_call_frame();
    call_stack.get_locals().resize(10);

    /*
    print(fib(1, 1, 10));

    let fib : Function(int a, int b, int steps_remaining){
        if(steps_remaining == 0){
            return b;
        }

        return fib(b, a + b, steps_remaining - 1);
    }
    */

    FunctionTable.push_back({});
    FunctionTable[0].entry_ip = 6;
    FunctionTable[0].arg_count = 3;
    FunctionTable[0].local_count = 3;

    // x = 5
    code.push_back({OpCode::PushConst, 1}); // stack_push 1  //0
    code.push_back({OpCode::PushConst, 1}); // stack_push 1
    code.push_back({OpCode::PushConst, 10});

    code.push_back({OpCode::Call, 0}); // 3
    code.push_back({OpCode::Store, X});

    code.push_back({OpCode::Halt});             // 5

    // fib function
    code.push_back({OpCode::Load, 2}); // load steps_remaining into the stack // 6
    code.push_back({OpCode::JumpIfZero, 17});

    // If we have NOT jumped i.e. we wanna call again:
    code.push_back({OpCode::Load, 1}); // Load b into stack  // 8


    code.push_back({OpCode::Load, 0}); // 9
    code.push_back({OpCode::Load, 1});
    code.push_back({OpCode::Add}); // Load (a+b) into stack

    code.push_back({OpCode::Load, 2}); // 12
    code.push_back({OpCode::PushConst, -1});
    code.push_back({OpCode::Add}); // Load steps_remaining-1 into stack

    code.push_back({OpCode::Call, 0}); // 15: Call fib with args (b, a+b, steps_remaining-1)

    code.push_back({OpCode::Return}); // 16


    // If we HAVE jumped, we wanna return b
    code.push_back({OpCode::Load, 1}); // 17
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
        case OpCode::Jump:
            e_Jump(cur);
            break;
        case OpCode::JumpIfZero:
            e_JumpIfZero(cur);
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

void VM::e_Jump(const Instruction &cur){
    ip = cur.a - 1;
}

void VM::e_JumpIfZero(const Instruction &cur){
    Value top = work_stack_pop();

    if(top.integer == 0){
        ip = cur.a - 1;
    }
}

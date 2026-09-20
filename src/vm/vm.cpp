#include "vm/vm.hpp"
#include "vm/instruction.hpp"
#include <stdio.h>
#include <iostream>

#define X 0
#define Y 1

namespace {

bool values_equal(const Value& left, const Value& right) {
    if (left.type != right.type) {
        return false;
    }

    switch (left.type) {
        case ValueType::Int: return left.integer == right.integer;
        case ValueType::Bool: return left.boolean == right.boolean;
        case ValueType::Object: return left.objectId == right.objectId;
        case ValueType::Function: return left.functionId == right.functionId;
        case ValueType::Null: return true;
    }

    return false;
}

}  // namespace

void VM::run_program() {

    call_stack.add_call_frame();
    call_stack.get_locals().resize(program.local_count);

    for (std::size_t index = 0; index < program.functions.size(); ++index) {
        const BytecodeProgram::BytecodeFunction& function = program.functions[index];
        call_stack.get_global_locals()[function.global_index] =
            Value::Function(static_cast<std::uint32_t>(index));
    }

    ip = 0;
    while(ip < program.code.size()){
        Instruction cur = program.code[ip];

        execute_instruction(cur);
        ip ++;
    }

    //std::get<ArrayObject>(heap[2]).print();
}

void VM::execute_instruction(const Instruction &cur){
    switch(cur.op){
        case OpCode::PushConst:
            e_PushConst(cur);
            break;
        case OpCode::PushRef:
            e_PushRef(cur);
            break;
        case OpCode::Dup:
            e_Dup(cur);
            break;
        case OpCode::Pop:
            e_Pop(cur);
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
        case OpCode::ArrayGet:
            e_ArrayGet(cur);
            break;
        case OpCode::ArraySet:
            e_ArraySet(cur);
            break;
        case OpCode::ArrayLength:
            e_ArrayLength(cur);
            break;
        case OpCode::ArrayPush:
            e_ArrayPush(cur);
            break;
        case OpCode::Store:
            e_Store(cur);
            break;
        case OpCode::Add:
            e_Add(cur);
            break;
        case OpCode::Subtract:
            e_Subtract(cur);
            break;
        case OpCode::Multiply:
            e_Multiply(cur);
            break;
        case OpCode::Divide:
            e_Divide(cur);
            break;
        case OpCode::EQEQ:
            e_EQEQ(cur);
            break;
        case OpCode::NEQ:
            e_NEQ(cur);
            break;
        case OpCode::LessThan:
            e_LessThan(cur);
            break;
        case OpCode::LessEqual:
            e_LessEqual(cur);
            break;
        case OpCode::GreaterThan:
            e_GreaterThan(cur);
            break;
        case OpCode::GreaterEqual:
            e_GreaterEqual(cur);
            break;
        case OpCode::Call:
            e_Call(cur);
            break;
        case OpCode::CallIndirect:
            e_CallIndirect(cur);
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
            e_Print(cur);
            break;
        case OpCode::Halt:
            ip = 10000;
            break;
    }
}

void VM::e_PushConst(const Instruction &cur){
    work_stack.push_back(Value::Int(cur.a));
}

void VM::e_PushRef(const Instruction &cur){
    work_stack.push_back(Value::Object(cur.a));
}

void VM::e_Dup(const Instruction&){
    work_stack_push(work_stack_top());
}

void VM::e_Pop(const Instruction&){
    work_stack_pop();
}

void VM::e_Print(const Instruction&){
    Value value = work_stack_pop();

    if(value.type == ValueType::Int){
        value.print();
        std::cout << "\n";
    } else if(value.type == ValueType::Object){
        HeapObject& obj = heap[value.objectId];
        std::get<ArrayObject>(obj).print();
        std::cout << "\n";
    }
    work_stack_push(Value::Null());
}

void VM::e_Load(const Instruction &cur){
    // a = local index to load
    std::vector<Value>& locals =
        static_cast<std::size_t>(cur.a) < program.global_count
            ? call_stack.get_global_locals()
            : call_stack.get_locals();
    work_stack.push_back(locals[cur.a]);
}

void VM::e_ArrayPushBack(const Instruction &cur){
    // Push top element of the stack to the array being referenced.
    // Local variables store an array referenced.
    Value val = work_stack_pop();
    Value array_reference = work_stack_pop();

    HeapObject& obj = heap[array_reference.objectId];

    std::get<ArrayObject>(obj).elements.push_back(val);

}

void VM::e_Store(const Instruction &cur){
    Value item = work_stack_pop();

    //std::cout << ref.objectId << "|----";

    std::vector<Value>& locals =
        static_cast<std::size_t>(cur.a) < program.global_count
            ? call_stack.get_global_locals()
            : call_stack.get_locals();
    locals[cur.a] = item;
}

void VM::e_PushNewArray(const Instruction &cur){
    uint32_t objectId = static_cast<uint32_t>(heap.size());
    
    heap.push_back(ArrayObject{});
    
    work_stack.push_back(Value::Object(objectId));
}

void VM::e_ArrayGet(const Instruction &cur){
    const Value index = work_stack_pop();
    const Value array_reference = work_stack_pop();

    if (index.type != ValueType::Int) {
        throw std::runtime_error("array index must be an integer");
    }
    if (array_reference.type != ValueType::Object) {
        throw std::runtime_error("attempted to index a value that is not an array");
    }
    if (array_reference.objectId >= heap.size()) {
        throw std::runtime_error("array reference is invalid");
    }

    HeapObject& obj = heap[array_reference.objectId];
    if (!std::holds_alternative<ArrayObject>(obj)) {
        throw std::runtime_error("attempted to index a value that is not an array");
    }

    const std::vector<Value>& elements = std::get<ArrayObject>(obj).elements;
    if (
        index.integer < 0 ||
        static_cast<std::size_t>(index.integer) >= elements.size()
    ) {
        throw std::runtime_error("array index is out of bounds");
    }

    work_stack_push(elements[static_cast<std::size_t>(index.integer)]);
}

void VM::e_ArraySet(const Instruction&){
    const Value value = work_stack_pop();
    const Value index = work_stack_pop();
    const Value array_reference = work_stack_pop();

    if (index.type != ValueType::Int) {
        throw std::runtime_error("array index must be an integer");
    }
    if (array_reference.type != ValueType::Object) {
        throw std::runtime_error("attempted to index a value that is not an array");
    }
    if (array_reference.objectId >= heap.size()) {
        throw std::runtime_error("array reference is invalid");
    }

    HeapObject& object = heap[array_reference.objectId];
    if (!std::holds_alternative<ArrayObject>(object)) {
        throw std::runtime_error("attempted to index a value that is not an array");
    }

    std::vector<Value>& elements = std::get<ArrayObject>(object).elements;
    if (
        index.integer < 0 ||
        static_cast<std::size_t>(index.integer) >= elements.size()
    ) {
        throw std::runtime_error("array index is out of bounds");
    }

    elements[static_cast<std::size_t>(index.integer)] = value;
}

void VM::e_ArrayLength(const Instruction&){
    const Value array_reference = work_stack_pop();

    if (array_reference.type != ValueType::Object) {
        throw std::runtime_error("len() requires an array");
    }
    if (array_reference.objectId >= heap.size()) {
        throw std::runtime_error("array reference is invalid");
    }

    const HeapObject& object = heap[array_reference.objectId];
    if (!std::holds_alternative<ArrayObject>(object)) {
        throw std::runtime_error("len() requires an array");
    }

    const auto& elements = std::get<ArrayObject>(object).elements;
    work_stack_push(Value::Int(static_cast<std::int64_t>(elements.size())));
}

void VM::e_ArrayPush(const Instruction&){
    const Value element = work_stack_pop();
    const Value array_reference = work_stack_pop();

    if (array_reference.type != ValueType::Object) {
        throw std::runtime_error("push() requires an array as its first argument");
    }
    if (array_reference.objectId >= heap.size()) {
        throw std::runtime_error("array reference is invalid");
    }

    HeapObject& object = heap[array_reference.objectId];
    if (!std::holds_alternative<ArrayObject>(object)) {
        throw std::runtime_error("push() requires an array as its first argument");
    }

    std::get<ArrayObject>(object).elements.push_back(element);
    work_stack_push(Value::Null());
}

void VM::e_Add(const Instruction &cur){
    Value a1 = work_stack_pop();
    Value a2 = work_stack_pop();

    if(a1.type == ValueType::Int && a2.type == ValueType::Int){
        Value a3 = Value::Int(a1.integer + a2.integer);
        work_stack_push(a3);
    } else {
        std::cerr << "addition (+) operator undefined for given type";
    }
}

void VM::e_Subtract(const Instruction &cur){
    Value a1 = work_stack_pop();
    Value a2 = work_stack_pop();

    if(a1.type == ValueType::Int && a2.type == ValueType::Int){
        Value a3 = Value::Int(a2.integer - a1.integer);
        work_stack_push(a3);
    } else {
        std::cerr << "subtraction (-) operator undefined for given type";
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

void VM::e_Divide(const Instruction &cur){
    Value a1 = work_stack_pop();
    Value a2 = work_stack_pop();

    if(a1.type == ValueType::Int && a2.type == ValueType::Int){
        Value a3 = Value::Int(a2.integer / a1.integer);
        work_stack_push(a3);
    } else {
        std::cerr << "division (/) operator undefined for given type";
    }
}

void VM::e_EQEQ(const Instruction &cur){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();

    work_stack_push(Value::Int(values_equal(left, right) ? 1 : 0));
}

void VM::e_NEQ(const Instruction &cur){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();

    work_stack_push(Value::Int(values_equal(left, right) ? 0 : 1));
}

void VM::e_LessThan(const Instruction&){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();
    if (left.type != ValueType::Int || right.type != ValueType::Int) {
        throw std::runtime_error("'<' requires integer operands");
    }
    work_stack_push(Value::Int(left.integer < right.integer ? 1 : 0));
}

void VM::e_LessEqual(const Instruction&){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();
    if (left.type != ValueType::Int || right.type != ValueType::Int) {
        throw std::runtime_error("'<=' requires integer operands");
    }
    work_stack_push(Value::Int(left.integer <= right.integer ? 1 : 0));
}

void VM::e_GreaterThan(const Instruction&){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();
    if (left.type != ValueType::Int || right.type != ValueType::Int) {
        throw std::runtime_error("'>' requires integer operands");
    }
    work_stack_push(Value::Int(left.integer > right.integer ? 1 : 0));
}

void VM::e_GreaterEqual(const Instruction&){
    const Value right = work_stack_pop();
    const Value left = work_stack_pop();
    if (left.type != ValueType::Int || right.type != ValueType::Int) {
        throw std::runtime_error("'>=' requires integer operands");
    }
    work_stack_push(Value::Int(left.integer >= right.integer ? 1 : 0));
}

void VM::e_Call(const Instruction &cur){
    // cur.a = function ID
    callFunction(static_cast<std::size_t>(cur.a));
}

void VM::e_CallIndirect(const Instruction &cur){
    const Value callee = work_stack_pop();
    if (callee.type != ValueType::Function) {
        throw std::runtime_error("attempted to call a value that is not a function");
    }

    const std::size_t function_index = callee.functionId;
    if (function_index >= program.functions.size()) {
        throw std::runtime_error("function value has an invalid function ID");
    }

    const BytecodeProgram::BytecodeFunction& function =
        program.functions[function_index];
    if (static_cast<std::size_t>(cur.a) != function.arg_count) {
        throw std::runtime_error(
            "indirect function call has the wrong number of arguments"
        );
    }

    callFunction(function_index);
}

void VM::callFunction(const std::size_t function_index){
    BytecodeProgram::BytecodeFunction& fn = program.functions[function_index];

    call_stack.add_call_frame();
    call_stack.get_locals().resize(fn.local_count);
    call_stack.scope().return_address = ip;

    ip = fn.entry_ip - 1;

    for(std::size_t i = fn.arg_count; i > 0; --i){
        call_stack.get_locals()[fn.parameter_start + i - 1] = work_stack_pop();
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

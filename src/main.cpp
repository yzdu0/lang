#include "vm/vm.hpp"
#include "compiler/ast.hpp"
#include "compiler/compiler.hpp"
#include "compiler/tokenizer.hpp"
#include "compiler/parser.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

std::string_view opcode_name(OpCode opcode);

int print_file_ast(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "lang: could not open '" << path << "'\n";
        return 1;
    }

    const std::string source(
        std::istreambuf_iterator<char>{input},
        std::istreambuf_iterator<char>{});

    Tokenizer tokenizer(source);
    const std::vector<Token> tokens = tokenizer.tokenize();

    try {
        Parser parser(tokens);
        const std::unique_ptr<Program> program = parser.parse();
        print_ast(*program, std::cout);

        Compiler compiler;
        const BytecodeProgram bytecode = compiler.compileProgram(*program);

        std::cout << "Bytecode\n";
        for (std::size_t index = 0; index < bytecode.code.size(); ++index) {
            const Instruction instruction = bytecode.code[index];
            std::cout << index << ": " << opcode_name(instruction.op);

            if (
                instruction.op == OpCode::PushConst ||
                instruction.op == OpCode::Load ||
                instruction.op == OpCode::Store ||
                instruction.op == OpCode::JumpIfZero
            ) {
                std::cout << ' ' << instruction.a;
            }

            std::cout << '\n';
        }

        if (!bytecode.functions.empty()) {
            std::cout << "Functions\n";
            for (std::size_t index = 0; index < bytecode.functions.size(); ++index) {
                const BytecodeProgram::BytecodeFunction& function =
                    bytecode.functions[index];
                std::cout
                    << index
                    << ": entry " << function.entry_ip
                    << ", args " << function.arg_count
                    << ", locals " << function.local_count
                    << '\n';
            }
        }
    } catch (const ParseError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    } catch (const CompileError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

std::string_view opcode_name(const OpCode opcode) {
    switch (opcode) {
        case OpCode::PushConst: return "PushConst";
        case OpCode::Load: return "Load";
        case OpCode::Store: return "Store";
        case OpCode::Add: return "Add";
        case OpCode::Subtract: return "Subtract";
        case OpCode::Multiply: return "Multiply";
        case OpCode::Divide: return "Divide";
        case OpCode::EQEQ: return "EQEQ";
        case OpCode::NEQ: return "NEQ";
        case OpCode::Return: return "Return";
        case OpCode::JumpIfZero: return "JumpIfZero";
        case OpCode::Halt: return "Halt";
        default: return "Unknown";
    }
}

}  // namespace

int main(const int argc, char* argv[]) {
    if (argc == 3 && std::string_view(argv[1]) == "--ast") {
        return print_file_ast(argv[2]);
    }

    if (argc != 1) {
        std::cerr << "Usage: lang [--ast <source-file>]\n";
        return 1;
    }

    VM vm;
    vm.run_program();

    return 0;
}

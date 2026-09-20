#include "vm/vm.hpp"
#include "compiler/ast.hpp"
#include "compiler/compiler.hpp"
#include "compiler/tokenizer.hpp"
#include "compiler/parser.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

std::string_view opcode_name(OpCode opcode);

int process_file(const std::string& path, const bool execute) {
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
                instruction.op == OpCode::Call ||
                instruction.op == OpCode::CallIndirect ||
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
                    << index << " (" << function.name << ')'
                    << ": entry " << function.entry_ip
                    << ", args " << function.arg_count
                    << ", global " << function.global_index
                    << ", parameters " << function.parameter_start
                    << ", locals " << function.local_count
                << '\n';
            }
        }

        if (!bytecode.local_mappings.empty() || !bytecode.functions.empty()) {
            std::cout << "Local variables\n";

            if (!bytecode.local_mappings.empty()) {
                std::cout << "Program\n";
                for (const auto& local : bytecode.local_mappings) {
                    std::cout << "  " << local.name << " -> " << local.index << '\n';
                }
            }

            for (std::size_t index = 0; index < bytecode.functions.size(); ++index) {
                const auto& function = bytecode.functions[index];
                if (function.local_mappings.empty()) {
                    continue;
                }

                std::cout << "Function " << index << " (" << function.name << ")\n";
                for (const auto& local : function.local_mappings) {
                    std::cout << "  " << local.name << " -> " << local.index << '\n';
                }
            }
        }

        if (execute) {
            std::cout << "Output\n";
            VM vm(bytecode);
            vm.run_program();
        }
    } catch (const ParseError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    } catch (const CompileError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    } catch (const std::runtime_error& error) {
        std::cerr << "Runtime error: " << error.what() << '\n';
        return 1;
    }

    return 0;
}

std::string_view opcode_name(const OpCode opcode) {
    switch (opcode) {
        case OpCode::PushConst: return "PushConst";
        case OpCode::Dup: return "Dup";
        case OpCode::Pop: return "Pop";
        case OpCode::Load: return "Load";
        case OpCode::Store: return "Store";
        case OpCode::Add: return "Add";
        case OpCode::Subtract: return "Subtract";
        case OpCode::Multiply: return "Multiply";
        case OpCode::Divide: return "Divide";
        case OpCode::EQEQ: return "EQEQ";
        case OpCode::NEQ: return "NEQ";
        case OpCode::LessThan: return "LessThan";
        case OpCode::LessEqual: return "LessEqual";
        case OpCode::GreaterThan: return "GreaterThan";
        case OpCode::GreaterEqual: return "GreaterEqual";
        case OpCode::PushNewArray: return "PushNewArray";
        case OpCode::ArrayGet: return "ArrayGet";
        case OpCode::ArrayLength: return "ArrayLength";
        case OpCode::ArrayPush: return "ArrayPush";
        case OpCode::ArrayPushBack: return "ArrayPushBack";
        case OpCode::Call: return "Call";
        case OpCode::CallIndirect: return "CallIndirect";
        case OpCode::Return: return "Return";
        case OpCode::Print: return "Print";
        case OpCode::JumpIfZero: return "JumpIfZero";
        case OpCode::Halt: return "Halt";
        default: return "Unknown";
    }
}

}  // namespace

int main(const int argc, char* argv[]) {
    if (argc == 2) {
        return process_file(argv[1], true);
    }

    if (argc == 3 && std::string_view(argv[1]) == "--ast") {
        return process_file(argv[2], false);
    }

    std::cerr << "Usage: lang <source-file>\n"
              << "       lang --ast <source-file>\n";
    return 1;
}

#pragma once

#include "compiler/ast.hpp"
#include "vm/instruction.hpp"
#include "vm/bytecode_program.hpp"

#include <cstddef>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

class CompileError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class Compiler {
public:
    BytecodeProgram compileProgram(const Program& program);

private:
    BytecodeProgram code;

    class StackLocals {
    public:
        StackLocals();

        void pushScope();
        void popScope();
        std::int32_t addLocal(const std::string& name);
        std::optional<std::int32_t> find(const std::string& name) const;
        std::size_t size() const;

    private:
        std::vector<std::unordered_map<std::string, std::int32_t>> scopes;
        std::int32_t next_local = 0;
        std::size_t max_local_count = 0;
    };

    StackLocals locals;
    std::unordered_map<std::string, std::size_t> functions;
    bool compiling_function = false;

    void emit(OpCode op);
    void emit(Instruction instruction);
    void compileStmt(const Stmt& statement);
    void compileBlockStatement(const BlockStmt& block_statement);
    void compileFunctionStatement(
        const FunctionStmt& function_statement,
        std::size_t function_index
    );
    void compileExpr(const Expr& expression);
};

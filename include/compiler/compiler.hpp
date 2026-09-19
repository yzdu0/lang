#pragma once

#include "compiler/ast.hpp"
#include "vm/instruction.hpp"
#include "vm/bytecode_program.hpp"

#include <cstdint>
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
    std::unordered_map<std::string, std::int32_t> locals;

    void emit(OpCode op);
    void emit(Instruction instruction);
    void compileStmt(const Stmt& statement);
    void compileBlockStatement(const BlockStmt& block_statement);
    void compileExpr(const Expr& expression);
};

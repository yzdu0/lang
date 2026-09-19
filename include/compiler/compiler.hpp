#pragma once
#include <vector>
#include "instruction.hpp"
#include "ast.hpp"


class Compiler {
    std::vector<Instruction> compileProgram(Program program);

    std::vector<Instruction> compileStmt(Stmt statement);

    std::vector<Instruction> compileExpr(Expr expression);
};
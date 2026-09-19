#include "compiler/compiler.hpp"

#include "compiler/token.hpp"

#include <charconv>
#include <cstdint>
#include <system_error>

void Compiler::emit(const OpCode op) {
    code.code.push_back({op});
}

void Compiler::emit(const Instruction instruction) {
    code.code.push_back(instruction);
}

BytecodeProgram Compiler::compileProgram(const Program& program) {
    code = BytecodeProgram{};
    locals.clear();

    for (const auto& statement : program.statements) {
        compileStmt(*statement);
    }

    code.local_count = locals.size();
    emit(OpCode::Halt);
    return code;
}

void Compiler::compileStmt(const Stmt& statement) {
    if (const auto* block = dynamic_cast<const BlockStmt*>(&statement)) {
        compileBlockStatement(*block);
        return;
    }

    if (const auto* let = dynamic_cast<const LetStmt*>(&statement)) {
        const std::string name(let->name.lexeme);
        if (locals.contains(name)) {
            throw CompileError("Variable '" + name + "' is already declared.");
        }

        const auto local = static_cast<std::int32_t>(locals.size());
        locals.emplace(name, local);
        compileExpr(*let->initializer);
        emit({OpCode::Store, local});
        return;
    }

    if (const auto* assignment = dynamic_cast<const AssignmentStmt*>(&statement)) {
        const std::string name(assignment->name.lexeme);
        const auto local = locals.find(name);
        if (local == locals.end()) {
            throw CompileError("Variable '" + name + "' is not declared.");
        }

        compileExpr(*assignment->initializer);
        emit({OpCode::Store, local->second});
        return;
    }

    if (const auto* ifStmt = dynamic_cast<const IfStmt*>(&statement)) {
        compileExpr(*ifStmt->conditional);

        const std::size_t jump_index = code.code.size();
        emit(OpCode::JumpIfZero);

        compileBlockStatement(*ifStmt->body_);
        code.code[jump_index].a = static_cast<std::int32_t>(code.code.size());
        return;
    }



    throw CompileError("Statement cannot be compiled yet.");
}

void Compiler::compileBlockStatement(const BlockStmt& block_statement) {
    for (const auto& statement : block_statement.statements) {
        compileStmt(*statement);
    }
}

void Compiler::compileExpr(const Expr& expression) {
    if (const auto* integer = dynamic_cast<const IntegerExpr*>(&expression)) {
        std::int32_t value = 0;
        const char* begin = integer->token.lexeme.data();
        const char* end = begin + integer->token.lexeme.size();
        const auto result = std::from_chars(begin, end, value);

        if (result.ec != std::errc{} || result.ptr != end) {
            throw CompileError("Integer literal is outside the bytecode range.");
        }

        emit({OpCode::PushConst, value});
        return;
    }

    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expression)) {
        compileExpr(*binary->left);
        compileExpr(*binary->right);

        switch (binary->op.kind) {
            case TokenKind::EqualEqual:
                emit(OpCode::EQEQ);
                return;
            case TokenKind::BangEqual:
                emit(OpCode::NEQ);
                return;
            case TokenKind::Plus:
                emit(OpCode::Add);
                return;
            case TokenKind::Minus:
                emit(OpCode::Subtract);
                return;
            case TokenKind::Star:
                emit(OpCode::Multiply);
                return;
            case TokenKind::Slash:
                emit(OpCode::Divide);
                return;
            default:
                throw CompileError("Unsupported binary operator.");
        }
    }

    if (const auto* variable = dynamic_cast<const VariableExpr*>(&expression)) {
        const std::string name(variable->name.lexeme);
        const auto local = locals.find(name);
        if (local == locals.end()) {
            throw CompileError("Variable '" + name + "' is not declared.");
        }

        emit({OpCode::Load, local->second});
        return;
    }

    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expression)) {
        if (unary->op.kind != TokenKind::Minus) {
            throw CompileError("Unsupported unary operator.");
        }

        emit({OpCode::PushConst, 0});
        compileExpr(*unary->right);
        emit(OpCode::Subtract);
        return;
    }

    if (const auto* array = dynamic_cast<const ArrayExpr*>(&expression)) {

        return;
    }

    throw CompileError("Expression cannot be compiled yet.");
}

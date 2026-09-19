#include "compiler/compiler.hpp"

#include "compiler/token.hpp"

#include <algorithm>
#include <charconv>
#include <cstdint>
#include <system_error>

Compiler::StackLocals::StackLocals() {
    pushScope();
}

void Compiler::StackLocals::pushScope() {
    scopes.emplace_back();
}

void Compiler::StackLocals::popScope() {
    next_local -= static_cast<std::int32_t>(scopes.back().size());
    scopes.pop_back();
}

std::int32_t Compiler::StackLocals::addLocal(const std::string& name) {
    auto& scope = scopes.back();
    if (scope.contains(name)) {
        throw CompileError("Variable '" + name + "' is already declared in this scope.");
    }

    const std::int32_t local = next_local++;
    scope.emplace(name, local);
    max_local_count = std::max(
        max_local_count,
        static_cast<std::size_t>(next_local)
    );
    return local;
}

std::optional<std::int32_t> Compiler::StackLocals::find(
    const std::string& name
) const {
    for (auto scope = scopes.rbegin(); scope != scopes.rend(); ++scope) {
        const auto local = scope->find(name);
        if (local != scope->end()) {
            return local->second;
        }
    }

    return std::nullopt;
}

std::size_t Compiler::StackLocals::size() const {
    return max_local_count;
}

void Compiler::emit(const OpCode op) {
    code.code.push_back({op});
}

void Compiler::emit(const Instruction instruction) {
    code.code.push_back(instruction);
}

BytecodeProgram Compiler::compileProgram(const Program& program) {
    code = BytecodeProgram{};

    locals = StackLocals{};

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

        const std::int32_t local = locals.addLocal(name);

        compileExpr(*let->initializer);
        emit({OpCode::Store, local});
        return;
    }

    if (const auto* assignment = dynamic_cast<const AssignmentStmt*>(&statement)) {
        const std::string name(assignment->name.lexeme);
        const auto local = locals.find(name);
        if (!local) {
            throw CompileError("Variable '" + name + "' is not declared.");
        }

        compileExpr(*assignment->initializer);
        emit({OpCode::Store, *local});
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
    locals.pushScope();

    for (const auto& statement : block_statement.statements) {
        compileStmt(*statement);
    }

    locals.popScope();
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
        if (!local) {
            throw CompileError("Variable '" + name + "' is not declared.");
        }

        emit({OpCode::Load, *local});
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

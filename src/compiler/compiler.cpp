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
    functions.clear();
    compiling_function = false;

    std::vector<const FunctionStmt*> function_statements;

    for (const auto& statement : program.statements) {
        const auto* function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (!function) {
            continue;
        }

        const std::string name(function->name.lexeme);
        if (functions.contains(name)) {
            throw CompileError("Function '" + name + "' is already declared.");
        }

        const std::size_t function_index = code.functions.size();
        functions.emplace(name, function_index);
        code.functions.push_back({
            0,
            function->type->parameters.size(),
            0
        });
        function_statements.push_back(function);
    }

    for (const auto& statement : program.statements) {
        if (dynamic_cast<const FunctionStmt*>(statement.get())) {
            continue;
        }

        compileStmt(*statement);
    }

    code.local_count = locals.size();
    emit(OpCode::Halt);

    for (std::size_t index = 0; index < function_statements.size(); ++index) {
        compileFunctionStatement(*function_statements[index], index);
    }

    return code;
}

void Compiler::compileLetPrimitive(const LetStmt& let) {
    const std::string name(let.name.lexeme);

    const std::int32_t local = locals.addLocal(name);

    compileExpr(*let.initializer);
    emit({OpCode::Store, local});
}

void Compiler::compileLetArray(const LetStmt& let) {
    const std::string name(let.name.lexeme);
    const std::int32_t local = locals.addLocal(name);

    const auto* array_expression =
        dynamic_cast<const ArrayExpr*>(let.initializer.get());
    if (!array_expression) {
        throw CompileError(
            "Array variable '" + name + "' must be initialized with an array."
        );
    }

    compileExpr(*array_expression);
    emit({OpCode::Store, local});
}

void Compiler::compileStmt(const Stmt& statement) {
    if (const auto* block = dynamic_cast<const BlockStmt*>(&statement)) {
        compileBlockStatement(*block);
        return;
    }

    if (const auto* let = dynamic_cast<const LetStmt*>(&statement)) {
        if (!let->declaredType || let->declaredType->kind == TypeKind::Int) {
            compileLetPrimitive(*let);
            return;
        }

        if (let->declaredType->kind == TypeKind::Array) {
            compileLetArray(*let);
            return;
        }

        throw CompileError("Variable type cannot be compiled yet.");
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

    if (const auto* return_statement = dynamic_cast<const ReturnStmt*>(&statement)) {
        if (!compiling_function) {
            throw CompileError("'return' can only be used inside a function.");
        }

        compileExpr(*return_statement->value);
        emit(OpCode::Return);
        return;
    }

    if (const auto* expression_statement = dynamic_cast<const ExpressionStmt*>(&statement)) {
        compileExpr(*expression_statement->expression);
        emit(OpCode::Pop);
        return;
    }

    if (dynamic_cast<const FunctionStmt*>(&statement)) {
        throw CompileError("Function declarations must be at the top level.");
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

void Compiler::compileFunctionStatement(
    const FunctionStmt& function_statement,
    const std::size_t function_index
) {
    locals = StackLocals{};
    compiling_function = true;

    for (const auto& parameter : function_statement.type->parameters) {
        locals.addLocal(std::string(parameter.name.lexeme));
    }

    BytecodeProgram::BytecodeFunction& function = code.functions[function_index];
    function.entry_ip = code.code.size();

    compileBlockStatement(*function_statement.body);

    if (
        function_statement.body->statements.empty() ||
        !dynamic_cast<const ReturnStmt*>(
            function_statement.body->statements.back().get()
        )
    ) {
        throw CompileError(
            "Function '" + std::string(function_statement.name.lexeme) +
            "' must end with a return statement."
        );
    }

    function.local_count = locals.size();
    compiling_function = false;
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

    if (const auto* call = dynamic_cast<const CallExpr*>(&expression)) {
        const auto* callee = dynamic_cast<const VariableExpr*>(call->callee.get());
        if (!callee) {
            throw CompileError("Only named functions can be called.");
        }

        const std::string name(callee->name.lexeme);

        if (name == "print") {
            if (call->arguments.size() != 1) {
                throw CompileError(
                    "Function 'print' expects 1 argument, but got " +
                    std::to_string(call->arguments.size()) + "."
                );
            }

            compileExpr(*call->arguments.front());
            emit(OpCode::Print);
            return;
        }

        const auto function = functions.find(name);
        if (function == functions.end()) {
            throw CompileError("Function '" + name + "' is not declared.");
        }

        const std::size_t function_index = function->second;
        const std::size_t expected_arguments =
            code.functions[function_index].arg_count;
        if (call->arguments.size() != expected_arguments) {
            throw CompileError(
                "Function '" + name + "' expects " +
                std::to_string(expected_arguments) + " arguments, but got " +
                std::to_string(call->arguments.size()) + "."
            );
        }

        for (const auto& argument : call->arguments) {
            compileExpr(*argument);
        }

        emit({OpCode::Call, static_cast<std::int32_t>(function_index)});
        return;
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
        emit(OpCode::PushNewArray);

        for (const auto& element : array->elements) {
            emit(OpCode::Dup);
            compileExpr(*element);
            emit(OpCode::ArrayPushBack);
        }

        return;
    }

    throw CompileError("Expression cannot be compiled yet.");
}

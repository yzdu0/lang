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
    local_mappings.push_back({name, local});
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

std::size_t Compiler::StackLocals::activeSize() const {
    return static_cast<std::size_t>(next_local);
}

const std::vector<BytecodeProgram::LocalMapping>&
Compiler::StackLocals::mappings() const {
    return local_mappings;
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
    //functions.clear();
    compiling_function = false;

    /*
    1. Set up functions
    */

    std::vector<const FunctionStmt*> function_statements;

    for (const auto& statement : program.statements) {
        // Function statement is the entire function declaration.
        const auto* function = dynamic_cast<const FunctionStmt*>(statement.get());
        if (!function) {
            continue;
        }

        const std::string name(function->name.lexeme);
        if (locals.find(name)) {
            throw CompileError("Function '" + name + "' is already declared.");
        }

        /* When a function is declared, 

        */
        const std::int32_t global_index = locals.addLocal(name);
        code.functions.emplace_back();
        code.functions.back().arg_count = function->type->parameters.size();
        code.functions.back().global_index =
            static_cast<std::size_t>(global_index);
        code.functions.back().name = name;
        function_statements.push_back(function);
    }

    for (const auto& statement : program.statements) {
        if (dynamic_cast<const FunctionStmt*>(statement.get())) {
            continue;
        }

        compileStmt(*statement);
    }

    code.local_count = locals.size();
    code.local_mappings = locals.mappings();
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
        if (!let->declaredType) {
            compileLetPrimitive(*let);
            return;
        }

        const auto* inherent_type =
            dynamic_cast<const InherentType*>(let->declaredType.get());
        if (!inherent_type) {
            throw CompileError("Variable type cannot be compiled yet.");
        }

        switch (inherent_type->kind) {
            case TypeKind::Int:
                compileLetPrimitive(*let);
                return;
            case TypeKind::Array:
                compileLetArray(*let);
                return;
        }
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

void Compiler::compileFunctionBlockStatement(const BlockStmt& block_statement) {

    for (const auto& statement : block_statement.statements) {
        compileStmt(*statement);
    }

}

void Compiler::compileFunctionStatement(
    const FunctionStmt& function_statement,
    const std::size_t function_index
) {
    const std::size_t first_local_mapping = locals.mappings().size();
    locals.pushScope();
    compiling_function = true;

    BytecodeProgram::BytecodeFunction& function = code.functions[function_index];
    function.parameter_start = locals.activeSize();

    for (const auto& parameter : function_statement.type->parameters) {
        locals.addLocal(std::string(parameter.name.lexeme));
    }

    function.entry_ip = code.code.size();

    compileFunctionBlockStatement(*function_statement.body);

    // Ensure return statement is present
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

    function.local_mappings.assign(
        locals.mappings().begin() + first_local_mapping,
        locals.mappings().end()
    );
    function.local_count = code.local_count;
    for (const auto& local : function.local_mappings) {
        function.local_count = std::max(
            function.local_count,
            static_cast<std::size_t>(local.index) + 1
        );
    }
    compiling_function = false;
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

    if (const auto* call = dynamic_cast<const CallExpr*>(&expression)) {
        const auto* callee = dynamic_cast<const VariableExpr*>(call->callee.get());
        if (!callee) {
            throw CompileError("Only named functions can be called.");
        }

        const std::string name(callee->name.lexeme);
        /*
        Once we know the name of the function being called, this name should correspond to a
        local variable. This local variable is of the type Function (containing FunctionID)
        that inherits from the Value type.
        */

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

        const auto global = locals.find(name);
        if (!global) {
            throw CompileError("Function '" + name + "' is not declared.");
        }

        const auto function = std::find_if(
            code.functions.begin(),
            code.functions.end(),
            [global](const BytecodeProgram::BytecodeFunction& candidate) {
                return candidate.global_index == static_cast<std::size_t>(*global);
            }
        );
        if (function == code.functions.end()) {
            for (const auto& argument : call->arguments) {
                compileExpr(*argument);
            }
            compileExpr(*call->callee);
            emit({
                OpCode::CallIndirect,
                static_cast<std::int32_t>(call->arguments.size())
            });
            return;
        }

        const std::size_t function_index = static_cast<std::size_t>(
            std::distance(code.functions.begin(), function)
        );
        const std::size_t expected_arguments = function->arg_count;
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

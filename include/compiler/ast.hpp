#pragma once

#include "compiler/token.hpp"

#include <iosfwd>
#include <memory>
#include <utility>
#include <vector>

enum class TypeKind {
    Int,
    Array
};

struct Type {
    TypeKind kind;
    std::unique_ptr<Type> elementType; // only used for Array

    explicit Type(TypeKind kind, std::unique_ptr<Type> elementType = nullptr)
        : kind(kind), elementType(std::move(elementType)) {}
};

struct Expr {
    virtual ~Expr() = default;
};

struct IntegerExpr final : Expr {
    explicit IntegerExpr(Token token) : token(token) {}
    Token token;
};

struct VariableExpr final : Expr {
    explicit VariableExpr(Token name) : name(name) {}
    Token name;
};

struct UnaryExpr final : Expr {
    UnaryExpr(Token op, std::unique_ptr<Expr> right)
        : op(op), right(std::move(right)) {}

    Token op;
    std::unique_ptr<Expr> right;
};

struct BinaryExpr final : Expr {
    BinaryExpr(
        std::unique_ptr<Expr> left,
        Token op,
        std::unique_ptr<Expr> right
    ) : left(std::move(left)), op(op), right(std::move(right)) {}

    std::unique_ptr<Expr> left;
    Token op;
    std::unique_ptr<Expr> right;
};

struct Stmt {
    virtual ~Stmt() = default;
};

struct LetStmt final : Stmt {
    LetStmt(
        Token name,
        std::unique_ptr<Type> declaredType,
        std::unique_ptr<Expr> initializer
    ) : name(name),
        declaredType(std::move(declaredType)),
        initializer(std::move(initializer)) {}

    Token name;
    std::unique_ptr<Type> declaredType;
    std::unique_ptr<Expr> initializer;
};

struct AssignmentStmt final : Stmt {
    AssignmentStmt(Token name, std::unique_ptr<Expr> initializer)
        : name(name), initializer(std::move(initializer)) {}

    Token name;
    std::unique_ptr<Expr> initializer;
};

struct ReturnStmt final : Stmt {
    explicit ReturnStmt(std::unique_ptr<Expr> value)
        : value(std::move(value)) {}

    std::unique_ptr<Expr> value;
};

struct ExpressionStmt final : Stmt {
    explicit ExpressionStmt(std::unique_ptr<Expr> expression)
        : expression(std::move(expression)) {}

    std::unique_ptr<Expr> expression;
};

struct BlockStmt final : Stmt {
    explicit BlockStmt(std::vector<std::unique_ptr<Stmt>> statements)
        : statements(std::move(statements)) {}

    std::vector<std::unique_ptr<Stmt>> statements;
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
    //std::vector<std::unique_ptr<Stmt>> statements;
};

void print_ast(const Program& program, std::ostream& output);

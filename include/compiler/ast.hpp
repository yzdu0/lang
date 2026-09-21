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
    virtual ~Type() = default;
};

struct FunctionParameter {
    FunctionParameter(Token name, std::unique_ptr<Type> type)
        : name(name), type(std::move(type)) {}

    explicit FunctionParameter(std::unique_ptr<Type> type)
        : name{}, type(std::move(type)) {}

    Token name;
    std::unique_ptr<Type> type;
};

struct InherentType final : Type {
    TypeKind kind;
    std::unique_ptr<Type> elementType; // only used for Array

    explicit InherentType(TypeKind kind, std::unique_ptr<Type> elementType = nullptr)
        : kind(kind), elementType(std::move(elementType)) {}
};

struct NamedType final : Type {
    NamedType(Token name, std::vector<std::unique_ptr<Type>> arguments)
        : name(name), arguments(std::move(arguments)) {}

    Token name;
    std::vector<std::unique_ptr<Type>> arguments;
};

struct FunctionType final : Type {
    FunctionType(
        std::vector<FunctionParameter> parameters,
        std::unique_ptr<Type> returnType
    ) : parameters(std::move(parameters)),
        returnType(std::move(returnType)) {}

    std::vector<FunctionParameter> parameters;
    std::unique_ptr<Type> returnType;
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

struct ArrayExpr final : Expr {
    explicit ArrayExpr(std::vector<std::unique_ptr<Expr>> elements)
        : elements(std::move(elements)) {}

    std::vector<std::unique_ptr<Expr>> elements;
};

struct CallExpr final : Expr {
    CallExpr(
        std::unique_ptr<Expr> callee,
        std::vector<std::unique_ptr<Expr>> arguments
    ) : callee(std::move(callee)), arguments(std::move(arguments)) {}

    std::unique_ptr<Expr> callee;
    std::vector<std::unique_ptr<Expr>> arguments;
};

struct ArrayLookExpr final : Expr {
    ArrayLookExpr(
        std::unique_ptr<Expr> array_variable,
        std::unique_ptr<Expr> array_index
    ) : array_variable(std::move(array_variable)), array_index(std::move(array_index)) {}
    
    std::unique_ptr<Expr> array_variable;
    std::unique_ptr<Expr> array_index;
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

struct StructStmt final : Stmt {
    struct TypeParameter {
        Token name;
    };

    struct Field {
        Token name;
        std::unique_ptr<Type> type;
    };

    StructStmt(
        Token name,
        std::vector<TypeParameter> type_parameters,
        std::vector<Field> fields
    ) : name(name),
        type_parameters(std::move(type_parameters)),
        fields(std::move(fields)) {}

    Token name;
    std::vector<TypeParameter> type_parameters;
    std::vector<Field> fields;
};

struct ArrayElementAssignmentStmt final : Stmt {
    ArrayElementAssignmentStmt(
        std::unique_ptr<ArrayLookExpr> target,
        std::unique_ptr<Expr> initializer
    ) : target(std::move(target)), initializer(std::move(initializer)) {}

    std::unique_ptr<ArrayLookExpr> target;
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

struct IfStmt final : Stmt {
    explicit IfStmt(
        std::unique_ptr<Expr> conditional,
        std::unique_ptr<BlockStmt> body_
    ) : conditional(std::move(conditional)), body_(std::move(body_)) {}

    std::unique_ptr<Expr> conditional;
    std::unique_ptr<BlockStmt> body_;
};

struct WhileStmt final : Stmt {
    explicit WhileStmt(
        std::unique_ptr<Expr> conditional,
        std::unique_ptr<BlockStmt> body_
    ) : conditional(std::move(conditional)), body_(std::move(body_)) {}

    std::unique_ptr<Expr> conditional;
    std::unique_ptr<BlockStmt> body_;
};

struct ForStmt final : Stmt {
    ForStmt(
        Token element_name,
        std::unique_ptr<Type> element_type,
        std::unique_ptr<Expr> iterable,
        std::unique_ptr<BlockStmt> body
    ) : element_name(element_name),
        element_type(std::move(element_type)),
        iterable(std::move(iterable)),
        body(std::move(body)) {}

    Token element_name;
    std::unique_ptr<Type> element_type;
    std::unique_ptr<Expr> iterable;
    std::unique_ptr<BlockStmt> body;
};

struct FunctionStmt final : Stmt {
    FunctionStmt(
        Token name,
        std::unique_ptr<FunctionType> type,
        std::unique_ptr<BlockStmt> body
    ) : name(name),
        type(std::move(type)),
        body(std::move(body)) {}

    Token name;
    std::unique_ptr<FunctionType> type;
    std::unique_ptr<BlockStmt> body;
};

struct Program {
    std::vector<std::unique_ptr<Stmt>> statements;
    //std::vector<std::unique_ptr<Stmt>> statements;
};

void print_ast(const Program& program, std::ostream& output);

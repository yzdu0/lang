#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "compiler/token.hpp"
#include "compiler/ast.hpp"

class ParseError : public std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class Parser
{
public:
    explicit Parser(const std::vector<Token> &tokens);

    std::unique_ptr<Program> parse();

private:
    const std::vector<Token> &tokens;
    std::size_t current = 0;

    std::unique_ptr<Type> parseType();
    std::unique_ptr<Type> parseArrayType();
    std::unique_ptr<Type> parsePrimitiveType();
    std::unique_ptr<Type> parseFunctionType();
    std::vector<std::unique_ptr<Type>> parseTypeList();
    // Statements
    std::unique_ptr<Stmt> parseStatement();
    std::unique_ptr<Stmt> parseLetStatement();
    std::unique_ptr<Stmt> parseAssignmentStatement();
    std::unique_ptr<Stmt> parseIfStatement();
    std::unique_ptr<Stmt> parseWhileStatement();
    std::unique_ptr<Stmt> parseForStatement();
    std::unique_ptr<Stmt> parseFunctionStatement();
    std::unique_ptr<Stmt> parseReturnStatement();
    std::unique_ptr<Stmt> parseExpressionStatement();
    std::unique_ptr<BlockStmt> parseBlockStatement();

    // Expressions
    std::unique_ptr<Expr> parseExpression();
    std::unique_ptr<Expr> parseArrayExpr();
    std::vector<std::unique_ptr<Expr>> parseExprList();
    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAdditionExpr();
    std::unique_ptr<Expr> parseMultiplicationExpr();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parseCall();
    std::unique_ptr<Expr> finishCall(std::unique_ptr<Expr> callee);
    std::unique_ptr<Expr> parsePrimary();
    std::unique_ptr<Expr> parseArrayLookExpr(std::unique_ptr<Expr> array_variable);

    // Token helpers
    bool match(TokenKind kind);
    bool check(TokenKind kind) const;
    bool checkNext(TokenKind kind) const;

    const Token &advance();
    const Token &peek() const;
    const Token &previous() const;

    const Token &consume(
        TokenKind kind,
        const std::string &message);

    bool isAtEnd() const;

    [[noreturn]]
    void error(
        const Token &token,
        const std::string &message) const;
};

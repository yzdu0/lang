#pragma once

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "token.hpp"
#include "ast.hpp"

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

    // Expressions
    std::unique_ptr<Expr> parseExpression();

    std::unique_ptr<Expr> parseEquality();
    std::unique_ptr<Expr> parseComparison();
    std::unique_ptr<Expr> parseAdditionExpr();
    std::unique_ptr<Expr> parseMultiplicationExpr();
    std::unique_ptr<Expr> parseUnary();
    std::unique_ptr<Expr> parsePrimary();

    // Token helpers
    bool match(TokenKind kind);
    bool check(TokenKind kind) const;

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

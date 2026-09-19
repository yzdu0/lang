#include "compiler/parser.hpp"

#include <utility>

Parser::Parser(const std::vector<Token> &tokens)
    : tokens(tokens) {}

std::unique_ptr<Program> Parser::parse()
{
    auto program = std::make_unique<Program>();

    while (!isAtEnd())
    {
        program->statements.push_back(
            parseStatement()
            // parseDeclaration()
        );
    }

    return program;
}


// TYPES

std::unique_ptr<Type> Parser::parseType(){
    if (check(TokenKind::ArrayType)) {
        return parseArrayType();
    }

    return parsePrimitiveType();
}

std::unique_ptr<Type> Parser::parseArrayType(){
    consume(TokenKind::ArrayType, "Expected 'array'.");
    consume(TokenKind::Less, "Expected '<' after 'array'.");
    std::unique_ptr<Type> elementType = parseType();
    consume(TokenKind::Greater, "Expected '>' after array element type.");

    return std::make_unique<Type>(
        TypeKind::Array,
        std::move(elementType)
    );
}

std::unique_ptr<Type> Parser::parsePrimitiveType(){
    if (match(TokenKind::IntType)) {
        return std::make_unique<Type>(TypeKind::Int);
    }

    error(peek(), "Expected type.");
}

// --------------------------------------------------
// Declarations / statements
// --------------------------------------------------
std::unique_ptr<Stmt> Parser::parseStatement(){
    if(match(TokenKind::Let)){
        return parseLetStatement();
    }

    if(check(TokenKind::Identifier)){
        return parseAssignmentStatement();
    }

    if(match(TokenKind::If)){
        return parseIfStatement();
    }

    if(match(TokenKind::LeftBrace)){
        return parseBlockStatement();
    }

    error(peek(), "Expected statement.");
}

std::unique_ptr<Stmt> Parser::parseLetStatement(){
    Token iden = consume(TokenKind::Identifier, "Expected identifier after let");

    std::unique_ptr<Type> declaredType;
    if (match(TokenKind::Colon)) {
        declaredType = parseType();
    }

    consume(TokenKind::Equal, "Expected = after declaration");

    std::unique_ptr<Expr> rhs = parseExpression();
    consume(TokenKind::Semicolon, "Expected ';' after declaration.");

    return std::make_unique<LetStmt>(
        iden,
        std::move(declaredType),
        std::move(rhs)
    );
}

std::unique_ptr<Stmt> Parser::parseAssignmentStatement(){
    /* identifier = expression */
    Token iden = consume(TokenKind::Identifier, "Expected identifier");
    consume(TokenKind::Equal, "Expected '=' after identifier.");

    std::unique_ptr<Expr> rhs = parseExpression();
    consume(TokenKind::Semicolon, "Expected ';' after assignment.");

    return std::make_unique<AssignmentStmt>(
        iden, std::move(rhs)
    );
}

std::unique_ptr<Stmt> Parser::parseIfStatement(){
    consume(TokenKind::LeftParen, "Expected '(' after 'if'.");

    std::unique_ptr<Expr> cond = parseExpression();

    consume(TokenKind::RightParen, "Expected ')' after if condition.");
    consume(TokenKind::LeftBrace, "Expected '{' before if body.");

    std::unique_ptr<BlockStmt> body = parseBlockStatement();

    return std::make_unique<IfStmt>(
        std::move(cond),
        std::move(body)
    );
}

std::unique_ptr<BlockStmt> Parser::parseBlockStatement(){
    std::vector<std::unique_ptr<Stmt>> statements;

    while(!check(TokenKind::RightBrace) && !isAtEnd()){
        statements.push_back(parseStatement());
    }

    consume(TokenKind::RightBrace, "Expected '}' after block.");

    return std::make_unique<BlockStmt>(std::move(statements));
}
// --------------------------------------------------
// Expressions
// --------------------------------------------------

std::unique_ptr<Expr> Parser::parseExpression()
{
    if(match(TokenKind::LeftBracket)){
        return parseArrayExpr();
    }

    return parseEquality();
}

std::unique_ptr<Expr> Parser::parseArrayExpr(){
    std::vector<std::unique_ptr<Expr>> elements = parseExprList();

    consume(TokenKind::RightBracket, "Expected ']' at end of array expression.");

    return std::make_unique<ArrayExpr>(std::move(elements));
}

std::vector<std::unique_ptr<Expr>> Parser::parseExprList(){
    std::vector<std::unique_ptr<Expr>> expressions;

    if(check(TokenKind::RightBracket)){
        return expressions;
    }

    expressions.push_back(parseExpression());

    while(match(TokenKind::Comma)){
        expressions.push_back(parseExpression());
    }

    return expressions;
}



std::unique_ptr<Expr> Parser::parseEquality()
{
    auto expr = parseComparison();

    while(
        check(TokenKind::EqualEqual) ||
        check(TokenKind::BangEqual)
    )
    {
        Token op = advance();
        auto right = parseComparison();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right));
    }

    return expr;

}

std::unique_ptr<Expr> Parser::parseComparison()
{
    /*
    A term would be the LHS of the comparison expression,
    so we can consume this first.
    */
    auto expr = parseAdditionExpr();

    /*
    If this is a comparison, we advance to this:

    Otherwise, if the expression is just a term, this would pass,
    and we return the term.
    */
    while (
        check(TokenKind::Less) ||
        check(TokenKind::LessEqual) ||
        check(TokenKind::Greater) ||
        check(TokenKind::GreaterEqual))
    {
        /*
        In the case of a comparison, this becomes a binary expression
        */
        Token op = advance();
        auto right = parseAdditionExpr();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseAdditionExpr()
{
    /*
    In the case of 1 * 2 + 3 * 4
    parseMultiplicationExpr will parse the 1*2 first.
    */
    auto expr = parseMultiplicationExpr();

    while (
        check(TokenKind::Plus) ||
        check(TokenKind::Minus))
    {
        Token op = advance();
        auto right = parseMultiplicationExpr();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseMultiplicationExpr()
{
    auto expr = parseUnary();

    while (
        check(TokenKind::Star) ||
        check(TokenKind::Slash))
    {
        Token op = advance();
        auto right = parseUnary();

        expr = std::make_unique<BinaryExpr>(
            std::move(expr),
            op,
            std::move(right));
    }

    return expr;
}

std::unique_ptr<Expr> Parser::parseUnary() {
    if (
        match(TokenKind::Bang) ||
        match(TokenKind::Minus)
    ) {
        Token op = previous();

        auto right = parseUnary();

        return std::make_unique<UnaryExpr>(
            op,
            std::move(right)
        );
    }

    return parsePrimary();
}

std::unique_ptr<Expr> Parser::parsePrimary() {
    if (match(TokenKind::Integer)) {
        return std::make_unique<IntegerExpr>(
            previous()
        );
    }

    if (match(TokenKind::Identifier)) {
        return std::make_unique<VariableExpr>(
            previous()
        );
    }

    if (match(TokenKind::LeftParen)) {
        auto expr = parseExpression();

        consume(
            TokenKind::RightParen,
            "Expected ')' after expression."
        );

        return expr;
    }

    error(
        peek(),
        "Expected expression."
    );
}
// --------------------------------------------------
// Token helpers
// --------------------------------------------------

bool Parser::match(const TokenKind kind)
{
    if (!check(kind))
    {
        return false;
    }

    advance();
    return true;
}

bool Parser::check(const TokenKind kind) const
{
    if (isAtEnd())
    {
        return kind == TokenKind::EndOfFile;
    }

    return peek().kind == kind;
}

const Token &Parser::advance()
{
    if (!isAtEnd())
    {
        current++;
    }

    return previous();
}

const Token &Parser::peek() const
{
    return tokens[current];
}

const Token &Parser::previous() const
{
    return tokens[current - 1];
}

const Token &Parser::consume(
    const TokenKind kind,
    const std::string &message)
{
    if (check(kind))
    {
        return advance();
    }

    error(peek(), message);
}

bool Parser::isAtEnd() const
{
    return peek().kind == TokenKind::EndOfFile;
}

void Parser::error(
    const Token &token,
    const std::string &message) const
{
    throw ParseError(
        "Line " +
        std::to_string(token.location.line) +
        ": " +
        message);
}

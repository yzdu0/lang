#pragma once

#include <cstddef>
#include <string_view>

enum class TokenKind {
    EndOfFile,
    Invalid,

    Identifier,
    Integer,
    String,

    IntType,
    ArrayType,

    Let,
    Fn,
    If,
    For,
    In,
    Return,
    Print,

    LeftParen,
    RightParen,
    LeftBracket,
    RightBracket,
    LeftBrace,
    RightBrace,
    Colon,
    Semicolon,
    Comma,
    Dot,

    Equal,
    EqualEqual,
    Bang,
    BangEqual,
    Less,
    LessEqual,
    Greater,
    GreaterEqual,
    Plus,
    Minus,
    Arrow,
    Star,
    Slash,
    Percent,
};

struct SourceLocation {
    std::size_t offset;
    std::size_t line;
    std::size_t column;
};

struct Token {
    TokenKind kind;
    std::string_view lexeme;
    SourceLocation location;
};

std::string_view token_kind_name(TokenKind kind);

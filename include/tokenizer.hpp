#pragma once

#include "token.hpp"

#include <cstddef>
#include <string_view>
#include <vector>

class Tokenizer {
public:
    explicit Tokenizer(std::string_view source);

    Token next();
    std::vector<Token> tokenize();

private:
    bool at_end() const;
    char peek(std::size_t distance = 0) const;
    char advance();
    bool match(char expected);

    void skip_ignored();
    Token make_token(TokenKind kind) const;
    Token identifier();
    Token integer();
    Token string();

    std::string_view source_;
    std::size_t current_ = 0;
    std::size_t line_ = 1;
    std::size_t column_ = 1;

    std::size_t token_start_ = 0;
    std::size_t token_line_ = 1;
    std::size_t token_column_ = 1;
};

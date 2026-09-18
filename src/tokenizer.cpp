#include "tokenizer.hpp"

#include <cctype>

namespace {

bool is_identifier_start(const char character) {
    const auto value = static_cast<unsigned char>(character);
    return std::isalpha(value) != 0 || character == '_';
}

bool is_identifier_continue(const char character) {
    const auto value = static_cast<unsigned char>(character);
    return std::isalnum(value) != 0 || character == '_';
}

TokenKind identifier_kind(const std::string_view text) {
    if (text == "let") return TokenKind::Let;
    if (text == "fn") return TokenKind::Fn;
    if (text == "for") return TokenKind::For;
    if (text == "in") return TokenKind::In;
    if (text == "return") return TokenKind::Return;
    if (text == "print") return TokenKind::Print;
    return TokenKind::Identifier;
}

}  // namespace

Tokenizer::Tokenizer(const std::string_view source) : source_(source) {}

Token Tokenizer::next() {
    skip_ignored();

    token_start_ = current_;
    token_line_ = line_;
    token_column_ = column_;

    if (at_end()) {
        return make_token(TokenKind::EndOfFile);
    }

    const char character = advance();

    if (is_identifier_start(character)) return identifier();
    if (std::isdigit(static_cast<unsigned char>(character)) != 0) return integer();

    switch (character) {
        case '(': return make_token(TokenKind::LeftParen);
        case ')': return make_token(TokenKind::RightParen);
        case '[': return make_token(TokenKind::LeftBracket);
        case ']': return make_token(TokenKind::RightBracket);
        case '{': return make_token(TokenKind::LeftBrace);
        case '}': return make_token(TokenKind::RightBrace);
        case ':': return make_token(TokenKind::Colon);
        case ';': return make_token(TokenKind::Semicolon);
        case ',': return make_token(TokenKind::Comma);
        case '.': return make_token(TokenKind::Dot);
        case '+': return make_token(TokenKind::Plus);
        case '*': return make_token(TokenKind::Star);
        case '/': return make_token(TokenKind::Slash);
        case '%': return make_token(TokenKind::Percent);
        case '=': return make_token(match('=') ? TokenKind::EqualEqual : TokenKind::Equal);
        case '!': return make_token(match('=') ? TokenKind::BangEqual : TokenKind::Bang);
        case '<': return make_token(match('=') ? TokenKind::LessEqual : TokenKind::Less);
        case '>': return make_token(match('=') ? TokenKind::GreaterEqual : TokenKind::Greater);
        case '-': return make_token(match('>') ? TokenKind::Arrow : TokenKind::Minus);
        case '"': return string();
        default: return make_token(TokenKind::Invalid);
    }
}

std::vector<Token> Tokenizer::tokenize() {
    std::vector<Token> tokens;

    while (true) {
        const Token token = next();
        tokens.push_back(token);
        if (token.kind == TokenKind::EndOfFile) break;
    }

    return tokens;
}

bool Tokenizer::at_end() const {
    return current_ >= source_.size();
}

char Tokenizer::peek(const std::size_t distance) const {
    const std::size_t position = current_ + distance;
    return position < source_.size() ? source_[position] : '\0';
}

char Tokenizer::advance() {
    const char character = source_[current_++];
    if (character == '\n') {
        ++line_;
        column_ = 1;
    } else {
        ++column_;
    }
    return character;
}

bool Tokenizer::match(const char expected) {
    if (at_end() || peek() != expected) return false;
    advance();
    return true;
}

void Tokenizer::skip_ignored() {
    while (!at_end()) {
        switch (peek()) {
            case ' ':
            case '\t':
            case '\r':
            case '\n':
                advance();
                break;
            case '#':
                while (!at_end() && peek() != '\n') advance();
                break;
            default:
                return;
        }
    }
}

Token Tokenizer::make_token(const TokenKind kind) const {
    return Token{
        kind,
        source_.substr(token_start_, current_ - token_start_),
        SourceLocation{token_start_, token_line_, token_column_},
    };
}

Token Tokenizer::identifier() {
    while (is_identifier_continue(peek())) advance();
    return make_token(identifier_kind(source_.substr(token_start_, current_ - token_start_)));
}

Token Tokenizer::integer() {
    while (std::isdigit(static_cast<unsigned char>(peek())) != 0) advance();
    return make_token(TokenKind::Integer);
}

Token Tokenizer::string() {
    bool escaped = false;

    while (!at_end() && peek() != '\n') {
        const char character = advance();
        if (character == '"' && !escaped) return make_token(TokenKind::String);
        escaped = character == '\\' && !escaped;
        if (character != '\\') escaped = false;
    }

    return make_token(TokenKind::Invalid);
}

#include "ast.hpp"

#include <ostream>
#include <string_view>

namespace {

void indent(std::ostream& output, const std::size_t depth) {
    for (std::size_t i = 0; i < depth; ++i) {
        output << "|--";
    }
}

void print_expression(
    const Expr& expression,
    std::ostream& output,
    const std::size_t depth
) {
    indent(output, depth);

    if (const auto* integer = dynamic_cast<const IntegerExpr*>(&expression)) {
        output << "Integer " << integer->token.lexeme << '\n';
        return;
    }

    if (const auto* variable = dynamic_cast<const VariableExpr*>(&expression)) {
        output << "Variable " << variable->name.lexeme << '\n';
        return;
    }

    if (const auto* unary = dynamic_cast<const UnaryExpr*>(&expression)) {
        output << "Unary " << unary->op.lexeme << '\n';
        print_expression(*unary->right, output, depth + 1);
        return;
    }

    if (const auto* binary = dynamic_cast<const BinaryExpr*>(&expression)) {
        output << "Binary " << binary->op.lexeme << '\n';
        print_expression(*binary->left, output, depth + 1);
        print_expression(*binary->right, output, depth + 1);
    }
}

void print_statement(
    const Stmt& statement,
    std::ostream& output,
    const std::size_t depth
) {
    indent(output, depth);

    if (const auto* let = dynamic_cast<const LetStmt*>(&statement)) {
        output << "Let " << let->name.lexeme << '\n';
        print_expression(*let->initializer, output, depth + 1);
        return;
    }

    if (const auto* return_statement = dynamic_cast<const ReturnStmt*>(&statement)) {
        output << "Return\n";
        print_expression(*return_statement->value, output, depth + 1);
        return;
    }

    if (const auto* expression = dynamic_cast<const ExpressionStmt*>(&statement)) {
        output << "Expression\n";
        print_expression(*expression->expression, output, depth + 1);
        return;
    }

    if (const auto* block = dynamic_cast<const BlockStmt*>(&statement)) {
        output << "Block\n";
        for (const auto& child : block->statements) {
            print_statement(*child, output, depth + 1);
        }
    }
}

}  // namespace

void print_ast(const Program& program, std::ostream& output) {
    output << "Program\n";
    for (const auto& statement : program.statements) {
        print_statement(*statement, output, 1);
    }
}

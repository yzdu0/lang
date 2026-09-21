#include "compiler/ast.hpp"

#include <ostream>
#include <string_view>

namespace {

void indent(std::ostream& output, const std::size_t depth) {
    for (std::size_t i = 0; i < depth; ++i) {
        output << "|--";
    }
}

void print_type(const Type& type, std::ostream& output) {
    if (const auto* inherent = dynamic_cast<const InherentType*>(&type)) {
        if (inherent->kind == TypeKind::Int) {
            output << "int";
            return;
        }

        output << "array<";
        print_type(*inherent->elementType, output);
        output << '>';
        return;
    }

    if (const auto* named = dynamic_cast<const NamedType*>(&type)) {
        output << named->name.lexeme;
        if (!named->arguments.empty()) {
            output << '<';
            for (std::size_t index = 0; index < named->arguments.size(); ++index) {
                if (index != 0) {
                    output << ", ";
                }
                print_type(*named->arguments[index], output);
            }
            output << '>';
        }
        return;
    }

    if (const auto* function = dynamic_cast<const FunctionType*>(&type)) {
        output << "fn(";
        for (std::size_t index = 0; index < function->parameters.size(); ++index) {
            if (index != 0) {
                output << ", ";
            }
            print_type(*function->parameters[index].type, output);
        }
        output << ") -> ";
        print_type(*function->returnType, output);
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
        return;
    }

    if (const auto* array = dynamic_cast<const ArrayExpr*>(&expression)) {
        output << "Array\n";
        for (const auto& element : array->elements) {
            print_expression(*element, output, depth + 1);
        }
        return;
    }

    if (const auto* call = dynamic_cast<const CallExpr*>(&expression)) {
        output << "Call\n";
        print_expression(*call->callee, output, depth + 1);
        for (const auto& argument : call->arguments) {
            print_expression(*argument, output, depth + 1);
        }
        return;
    }

    if (const auto* array_look = dynamic_cast<const ArrayLookExpr*>(&expression)) {
        output << "ArrayGet\n";
        print_expression(*array_look->array_variable, output, depth + 1);
        print_expression(*array_look->array_index, output, depth + 1);
        return;
    }

    if (dynamic_cast<const EmptyStructExpr*>(&expression)) {
        output << "EmptyStruct\n";
        return;
    }

    if (const auto* field = dynamic_cast<const FieldAccessExpr*>(&expression)) {
        output << "Field " << field->field.lexeme << '\n';
        print_expression(*field->object, output, depth + 1);
        return;
    }
}

void print_statement(
    const Stmt& statement,
    std::ostream& output,
    const std::size_t depth
) {
    indent(output, depth);

    if (const auto* struct_statement = dynamic_cast<const StructStmt*>(&statement)) {
        output << "Struct " << struct_statement->name.lexeme;
        if (!struct_statement->type_parameters.empty()) {
            output << '<';
            for (std::size_t index = 0;
                 index < struct_statement->type_parameters.size(); ++index) {
                if (index != 0) {
                    output << ", ";
                }
                output << struct_statement->type_parameters[index].name.lexeme
                       << ": Type";
            }
            output << '>';
        }
        output << '\n';

        for (const auto& field : struct_statement->fields) {
            indent(output, depth + 1);
            output << "Field " << field.name.lexeme << ": ";
            print_type(*field.type, output);
            output << '\n';
        }
        return;
    }

    if (const auto* let = dynamic_cast<const LetStmt*>(&statement)) {
        output << "Let " << let->name.lexeme;
        if (let->declaredType) {
            output << ": ";
            print_type(*let->declaredType, output);
        }
        output << '\n';
        print_expression(*let->initializer, output, depth + 1);
        return;
    }

    if (const auto* let = dynamic_cast<const AssignmentStmt*>(&statement)) {
        output << "Assignment " << let->name.lexeme << '\n';
        print_expression(*let->initializer, output, depth + 1);
        return;
    }

    if (const auto* assignment =
        dynamic_cast<const ArrayElementAssignmentStmt*>(&statement)) {
        output << "ArrayAssignment\n";
        print_expression(*assignment->target, output, depth + 1);
        print_expression(*assignment->initializer, output, depth + 1);
        return;
    }

    if (const auto* assignment = dynamic_cast<const FieldAssignmentStmt*>(&statement)) {
        output << "FieldAssignment\n";
        print_expression(*assignment->target, output, depth + 1);
        print_expression(*assignment->initializer, output, depth + 1);
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
        return;
    }

    if (const auto* if_statement = dynamic_cast<const IfStmt*>(&statement)) {
        output << "If\n";
        print_expression(*if_statement->conditional, output, depth + 1);
        print_statement(*if_statement->body_, output, depth + 1);
        return;
    }

    if (const auto* while_statement = dynamic_cast<const WhileStmt*>(&statement)) {
        output << "While\n";
        print_expression(*while_statement->conditional, output, depth + 1);
        print_statement(*while_statement->body_, output, depth + 1);
        return;
    }

    if (const auto* for_statement = dynamic_cast<const ForStmt*>(&statement)) {
        output << "For " << for_statement->element_name.lexeme << ": ";
        print_type(*for_statement->element_type, output);
        output << '\n';
        print_expression(*for_statement->iterable, output, depth + 1);
        print_statement(*for_statement->body, output, depth + 1);
        return;
    }

    if (const auto* function = dynamic_cast<const FunctionStmt*>(&statement)) {
        output << "Function " << function->name.lexeme << " -> ";
        print_type(*function->type->returnType, output);
        output << '\n';

        for (const auto& parameter : function->type->parameters) {
            indent(output, depth + 1);
            output << "Parameter " << parameter.name.lexeme << ": ";
            print_type(*parameter.type, output);
            output << '\n';
        }

        print_statement(*function->body, output, depth + 1);
    }
}

}  // namespace

void print_ast(const Program& program, std::ostream& output) {
    output << "Program\n";
    for (const auto& statement : program.statements) {
        print_statement(*statement, output, 1);
    }
}

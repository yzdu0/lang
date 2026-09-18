#include "vm.hpp"
#include "ast.hpp"
#include "tokenizer.hpp"
#include "parser.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <string_view>

namespace {

int print_file_ast(const std::string& path) {
    std::ifstream input(path, std::ios::binary);
    if (!input) {
        std::cerr << "lang: could not open '" << path << "'\n";
        return 1;
    }

    const std::string source(
        std::istreambuf_iterator<char>{input},
        std::istreambuf_iterator<char>{});

    Tokenizer tokenizer(source);
    const std::vector<Token> tokens = tokenizer.tokenize();

    try {
        Parser parser(tokens);
        const std::unique_ptr<Program> program = parser.parse();
        print_ast(*program, std::cout);
    } catch (const ParseError& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }

    return 0;
}

}  // namespace

int main(const int argc, char* argv[]) {
    if (argc == 3 && std::string_view(argv[1]) == "--ast") {
        return print_file_ast(argv[2]);
    }

    if (argc != 1) {
        std::cerr << "Usage: lang [--ast <source-file>]\n";
        return 1;
    }

    VM vm;
    vm.run_program();

    return 0;
}

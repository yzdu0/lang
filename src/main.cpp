#include "vm.hpp"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

int main() {
    /*if (argc != 2) {
        std::cerr << "Usage: lang <bytecode-file>\n";
        return 1;
}

    std::ifstream input(argv[1], std::ios::binary);
    if (!input) {
        std::cerr << "lang: could not open '" << argv[1] << "'\n";
        return 1;
    }*/

    /*const std::string bytecode(
        std::istreambuf_iterator<char>{input},
        std::istreambuf_iterator<char>{});*/

    VM vm;
    vm.run_program();

    return 0;
}

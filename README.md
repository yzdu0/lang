# lang

A minimal C++ bytecode VM project built with CMake. The current executable is a
development harness: it loads a bytecode text file and passes it to the empty
`VM::run_program` method.

## Build

```sh
cmake -S . -B build
cmake --build build
```

Pass the bytecode file to the executable:

```sh
./build/lang examples/bytecode.txt
```

Or build and run the bytecode harness in one command:

```sh
cmake --build build --target run-bytecode
```

`VM::run_program` intentionally does not execute any instructions yet. It is the
place to develop the bytecode loop.

## Project layout

```text
include/         VM and other C++ headers
src/             VM implementation and command-line entry point
examples/        bytecode.txt and language examples
editors/vscode/  VS Code syntax-highlighting extension
```

The current CMake target builds only `main.cpp` and `vm.cpp`. The tokenizer
files remain in the repository, but they are not part of the VM build.

## VS Code highlighting

A small VS Code extension lives in `editors/vscode`. To try it without
installing or publishing it, open that directory in VS Code and press `F5` to
launch an Extension Development Host. Files ending in `.lang` will then use the
included syntax highlighting.

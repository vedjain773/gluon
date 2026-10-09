# Gluon
A C-like compiler written in C++, featuring a custom frontend, intermediate representation (IR), and RISC-V backend. The compiler lowers source programs through its own IR and machine IR before generating RISC-V assembly, which can be assembled and executed using the RISC-V GNU toolchain and QEMU.

## Usage
Clone the repository
```bash
git clone https://github.com/vedjain773/gluon.git && cd gluon 
```

Build the project
```bash
cmake --build build
```

Compile source
```bash
./gluon input.c --print-asm -o output.s
```

Assemble the generated asm, and statically link it
```bash
riscv64-linux-gnu-as output.s -o output.o
riscv64-linux-gnu-gcc -static output.o -o output
```

Emulate the executable on qemu
```bash
qemu-riscv64 ./output
```

| Flag              | Description           |
|-------------------|-----------------------|
| --print-tokens    | Print tokens          |
| --print-ast       | Print AST             |
| --print-ir        | Print IR              |
| --print-mir       | Print machine IR      |
| --print-asm       | Print Risc-V64 assembly to a file |
| -o                | Set destination file name |

## Compiler Features
The compiler currently consists of a custom frontend and a RISC-V backend. The frontend lowers source programs into a typed, LLVM-inspired intermediate representation built around functions, basic blocks, and instructions.

The RISC-V backend then lowers this IR into a simpler machine-level representation. It models virtual registers, physical registers, immediates, and stack locations, and currently handles integer arithmetic, comparisons, memory operations, address generation, and function returns.

The backend makes use of a Chaitin-style graph colouring register allocator where in virtual registers from the MIR are represented as nodes in an interference graph which are then subsequently coloured through iterative graph simplification and colouring.

The backend emits RISC-V 64-bit assembly which can be assembled with the GNU RISC-V toolchain and executed using QEMU. The generated code is intentionally straightforward for now; register allocation and most optimizations have not been implemented yet.

Also check out [Quark](https://github.com/vedjain773/quark)!

## Supported language features
- Integer types and expressions
- Local variables
- Pointers
- Arrays (including multi-dimensional arrays)
- Control flow through if-else blocks and while loops
- Blocks and nested scopes (variable shadowing)
- Comparison and arithmetic operators
- Function calls

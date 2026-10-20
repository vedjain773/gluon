# Gluon
A C-like compiler written in C++, featuring a custom frontend, intermediate representation (IR), and RISC-V backend. The compiler lowers source programs through its own IR and machine IR before generating RISC-V assembly, which can be assembled and executed using the RISC-V GNU toolchain and QEMU.

## Usage
Clone the repository
```bash
git clone https://github.com/vedjain773/gluon.git && cd gluon 
```

Build the project
```bash
cmake -S . -B build
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
| --optimize        | Apply optimizations   |
| --print-mir       | Print machine IR      |
| -o                | Set destination file name |

### Prerequesites
- A C++-20 compiler
- CMake (version 3.20 or above)

Gluon solely relies on the C++ standard library to compile C code into riscv64 assembly, the generated assembly file can then be assembled with the GNU RISC-V toolchain and executed using QEMU.

## Compiler Features
The compiler consists of a custom frontend and a RISC-V backend. The frontend lowers source programs into a typed, LLVM-inspired intermediate representation built around functions, basic blocks, and instructions.

The compiler also makes use of LLVM-style passes to optimize the IR. These passes can be found inside the `src/IR/passes` directory. These include:
- Mem2Reg (similar to LLVM's mem2reg pass)
- Constant Propagation
- Dead Branch Removal

The RISC-V backend then lowers this IR into a simpler machine-level representation. It models virtual registers, physical registers, immediates, and stack locations, and currently handles integer arithmetic, comparisons, memory operations, address generation, and function returns.

The backend makes use of a Chaitin-style graph colouring register allocator wherein virtual registers from the MIR are represented as nodes in an interference graph which are then subsequently coloured through iterative graph simplification and colouring.

The allocator uses the argument (a0-a7), temporary (t0-t5) and callee-saved registers (s1-s11) for register allocation and reserves the t6 register for spilling registers to the stack.

The backend emits RISC-V 64-bit assembly which can be assembled with the GNU RISC-V toolchain and executed using QEMU.

Also check out [Quark](https://github.com/vedjain773/quark)!

## Supported language features
- Integer types and expressions
- Local variables
- Pointers
- Arrays (including multi-dimensional arrays)
- Control flow through if-else blocks and while loops
- Break and Continue statements
- Blocks and nested scopes (variable shadowing)
- Comparison and arithmetic operators
- Function calls

## Example

Given this program:

```c
int main() {
    int x = 4;
    int y = x + 1;

    if (y == 5) {
        return 10;
    } else {
        return 20;
    }
}
```

**Unoptimized IR** (`./gluon example.c --print-ir`)

```
;Module: test.c

define @main () -> int
entry: []
  y.0 = alloca int
  x.0 = alloca int
  store int 4 -> int* x.0
  x.1 = load int <- int* x.0
  add.0 = add int x.1, int 1
  store int add.0 -> int* y.0
  y.1 = load int <- int* y.0
  compEE.0 = eq int y.1, int 5
  ifcond.0 = neq int compEE.0, int 0
  br int ifcond.0, if.then, if.else

if.then: [entry]
  ret int 10

if.else: [entry]
  ret int 20

if.after: []
```

**Optimized IR** (`./gluon example.c --print-ir --optimize`)

```
;Module: test.c

define @main () -> int
entry: []
  br if.then

if.then: [entry]
  ret int 10
```

**Emitted Assembly [From Optimized IR]**
```
.text
.globl main
.type main, @function

main:
.LBB0:
  addi sp, sp, -16
  sd ra, 8(sp)
  sd s0, 0(sp)
  addi s0, sp, 16

.LBB1:
  li a0, 10

.LBB2:
  ld ra, 8(sp)
  ld s0, 0(sp)
  addi sp, sp, 16
  ret 
```

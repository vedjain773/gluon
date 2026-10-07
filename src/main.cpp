#include "frontend/parser/Parser.hpp"
#include "frontend/scanner/Scanner.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "utils/Scope.hpp"

#include "backend/riscv64/Emitter.hpp"
#include "backend/riscv64/FramePass.hpp"
#include "backend/riscv64/LowerPass.hpp"
#include "backend/riscv64/RegAlloc.hpp"

#include <fstream>
#include <iostream>

struct CIConfig {
    bool optimize = false;
    bool printAST = false;
    bool printTokens = false;
    bool printIR = false;
    bool printMIR = false;
    bool printASM = false;
    bool nc = false;
};

int main(int argc, char **argv) {

    CIConfig config;

    if (argc < 2) {
        std::cerr << "Usage: gluon <src> [flags] [-o <dest>]\n";
        return 1;
    }

    std::string filename = argv[1];
    std::string destname = std::format("{}.s", filename);

    for (int i = 2; i < argc; ++i) {
        std::string_view arg = argv[i];

        if (arg == "--print-tokens") {
            config.printTokens = true;
        } else if (arg == "--print-ast") {
            config.printAST = true;
        } else if (arg == "--print-ir") {
            config.printIR = true;
        } else if (arg == "--optimize") {
            config.optimize = true;
        } else if (arg == "--print-mir") {
            config.printMIR = true;
        } else if (arg == "--print-asm") {
            config.printASM = true;
        } else if (arg == "--no-compile") {
            config.nc = true;
        } else if (arg == "-o") {
            if (++i >= argc) {
                std::cerr << "error: -o requires an argument\n";
                return 1;
            }

            destname = argv[i];
        } else {
            std::cerr << "error: unknown argument: " << arg << '\n';
            return 1;
        }
    }

    getSourceLines(filename);

    Scanner scanner(filename);
    scanner.scanFile();
    scanner.scanProg();

    if (config.printTokens) {
        scanner.printTokens();
    }

    std::vector<Token> tokenlist = scanner.getTokenList();

    Parser parser(tokenlist);
    auto prog = parser.ParseProgram();
    prog->setFileName(filename);

    int noErr = prog->semAnalyse();

    if (config.printAST) {
        prog->printAST();
        std::cout << "\n";
    }

    int totalErrors = noErr + parser.numOfErrors;

    if (totalErrors > 0) {
        std::cerr << "Build failed with " << totalErrors << " error(s)\n";
        return -1;
    }

    
    prog->codegen();
    if (config.printIR) prog->printIR();

    if (config.nc) return 0;

    RISCV::LowerPass lp(prog->getModule());
    lp.lower();

#ifdef PRINT_MIR
    lp.print(std::cout);      
#endif

    RISCV::RegAlloc allocator(lp.getModule());
    allocator.allocate();

#ifdef PRINT_ITG
    allocator.printITFGraph(std::cout);      
#endif

    RISCV::FramePass fp(lp.getModule());
    fp.run();

    if (config.printMIR) lp.print(std::cout);

    std::ofstream outfile(destname);
    RISCV::Emitter emitter(lp.getModule(), outfile);

    if (config.printASM) emitter.emit();

    return 0;
}

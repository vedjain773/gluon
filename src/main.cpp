#include "frontend/parser/Parser.hpp"
#include "frontend/scanner/Scanner.hpp"
#include "nodes/Program.hpp"
#include "utils/Error.hpp"
#include "utils/Scope.hpp"

#include "backend/riscv64/LowerPass.hpp"

#include <iostream>

int main(int argc, char **argv) { 

    bool printAst = false;
    bool printTokens = false;
    bool printIR = false;
    std::string filename = "input.c";

    if (argc == 1) {
        std::cout << "Usage: \n";
        std::cout << "./gluon` <src> <flag> <dest?>\n";
        return 0;
    }

    filename = argv[1];

    for (int i = 2; i < argc; i++) {
        std::string flag = argv[i];

        if (flag == "--pt") {
            printTokens = true;
        } else if (flag == "--past") {
            printAst = true;
        } else if (flag == "--emit-ir") {
            printIR = true; 
        } else {
            std::cout << "Unknown Flag: " << argv[i] << "\n";
        }
    }

    getSourceLines(filename);

    Scanner scanner(filename);
    scanner.scanFile();
    scanner.scanProg();

    if (printTokens) {
        scanner.printTokens();
    }

    std::vector<Token> tokenlist = scanner.getTokenList();

    Parser parser(tokenlist);
    auto prog = parser.ParseProgram();
    prog->setFileName(filename);
        
    int noErr = prog->semAnalyse();

    if (printAst) {
        prog->printAST();
        std::cout << "\n";
    }

    int totalErrors = noErr + parser.numOfErrors;

    if (totalErrors > 0) {
        std::cout << "Build failed with " << totalErrors << " error(s)\n";
        return -1;
    }

    prog->codegen();

    if (printIR) prog->printIR();

    RISCV::LowerPass lp(prog->getModule());
    lp.lower();
    lp.print(std::cout);

    return 0;
}

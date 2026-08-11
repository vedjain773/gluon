#include "nodes/Program.hpp"
#include "visitors/Visitor.hpp"
#include <cstddef>
#include <iostream>

void Program::accept(Visitor &visitor) {
    visitor.visitProgram(*this);
}

void Program::add(std::unique_ptr<ExternalDecl> edecl) {
    root.push_back(std::move(edecl));
}

void Program::printAST() {
    PrintVisitor printvisitor;
    this->accept(printvisitor);
}

int Program::semAnalyse() {
    SemanticVisitor semvisitor;
    this->accept(semvisitor);
    return semvisitor.numOfErrors;
}

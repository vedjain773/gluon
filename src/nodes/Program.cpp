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

void Program::setFileName(const std::string &fileName) { name = fileName; }

Module *Program::getModule() { return cdgvis.module.get(); }

void Program::printAST() {
    PrintVisitor printvisitor;
    this->accept(printvisitor);
}

int Program::semAnalyse() {
    SemanticVisitor semvisitor;
    this->accept(semvisitor);
    return semvisitor.numOfErrors;
}

void Program::codegen() {
    cdgvis.initModule(name);

    for (size_t i = 0; i < root.size(); i++) {
        root[i]->codegen(cdgvis);
        cdgvis.clearTable();
    }    
}

void Program::printIR() {
    cdgvis.module->print(std::cout);
}

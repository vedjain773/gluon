#ifndef PROGRAM_H
#define PROGRAM_H

#include "nodes/ExternalDecl.hpp"
#include "nodes/Statement.hpp"
#include "visitors/Visitor.hpp"
#include "visitors/CodegenVis.hpp"
#include <memory>
#include <string>
#include <vector>

class Program {
  private:
    std::string name;
    CodegenVis cdgvis;

  public:
    std::vector<std::unique_ptr<ExternalDecl>> root;
    std::string fileName;

    void setFileName(const std::string &fileName);
    
    void accept(Visitor &visitor);
    void add(std::unique_ptr<ExternalDecl> edecl);

    Module *getModule();
    void printAST();
    int semAnalyse();

    void codegen();
    void printIR();
};

#endif

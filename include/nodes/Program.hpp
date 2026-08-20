#ifndef PROGRAM_H
#define PROGRAM_H

#include "nodes/ExternalDecl.hpp"
#include "nodes/Statement.hpp"
#include "visitors/Visitor.hpp"
#include <memory>
#include <string>
#include <vector>

class Program {
  private:
    std::string name;

  public:
    std::vector<std::unique_ptr<ExternalDecl>> root;
    std::string fileName;

    void setFileName(const std::string &fileName);
    void accept(Visitor &visitor);
    void add(std::unique_ptr<ExternalDecl> edecl);

    void printAST();
    void codegen();
    int semAnalyse();
};

#endif

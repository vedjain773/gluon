#ifndef CODEGEN_H
#define CODEGEN_H

#include "IR/Module.hpp"
#include "IR/Func.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Inst.hpp"
#include "IR/Value.hpp"

#include "IR/IRBuilder.hpp"

#include <memory>

class CodegenVis {
  private: 
    std::unordered_map<std::string, AllocaInst *> table;

  public:
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder> builder;

    void initModule(const std::string &fileName);
    void insertName(const std::string &name, AllocaInst *alloca);

    void clearTable();
    AllocaInst *lookup(std::string name);
};

#endif


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
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder> builder;
    std::unordered_map<std::string, AllocaInst *> table;

  public:
    void initModule(const std::string &fileName);
    void insertName(const std::string &name, AllocaInst *alloca);
    AllocaInst *lookup(std::string name);
};

#endif


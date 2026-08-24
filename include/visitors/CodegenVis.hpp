#ifndef CODEGEN_H
#define CODEGEN_H

#include "IR/Module.hpp"
#include "IR/Func.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Inst.hpp"
#include "IR/Value.hpp"

#include "IR/IRBuilder.hpp"

#include <memory>

enum class Operators;

struct OpConfig {
    Value *lhs;
    Value *rhs;
    Operators op;
    TypeKind *type;
};

class CodegenVis {
  private: 
    std::unordered_map<std::string, AllocaInst *> table;

  public:
    std::unique_ptr<Module> module;
    std::unique_ptr<IRBuilder> builder;

    void initModule(const std::string &fileName);
    void insertName(const std::string &name, AllocaInst *alloca);

    Value *handleBinOp(const OpConfig &opconfig);
    Value *handlePtrArith(const OpConfig &opconfig);

    void clearTable();
    AllocaInst *lookup(std::string name);
};

#endif


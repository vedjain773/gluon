#ifndef FUNC_H
#define FUNC_H

#include "IR/BasicBlock.hpp" 
#include <vector>
#include <memory>
#include <string>

class Module;

class Func {
  private:
    std::string name;
    Module *parent;
    TypeKind *returnType;
    std::vector<TypeKind*> paramTypes;
    std::vector<Arg*> args;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;

  public:
    Func(const std::string &name, Module *parent,
            TypeKind *retType, std::vector<TypeKind*> params);
    std::string getName();
    Module *getParent();
    TypeKind *getReturnType();

    std::vector<Arg*> &getArgs();
    Arg *getArg(unsigned i);

    BasicBlock *appendBasicBlock(const std::string &name);
    BasicBlock *appendBasicBlock(std::unique_ptr<BasicBlock> bb);

    BasicBlock *getEntryBlock();

    void print(std::ostream &os) const;
};

#endif

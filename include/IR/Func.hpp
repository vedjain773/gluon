#ifndef FUNC_H
#define FUNC_H

#include "IR/BasicBlock.hpp" 
#include <vector>
#include <memory>
#include <string>

class Func {
private:
    Module *parent;
    TypeKind *returnType;
    std::vector<Argument*> args;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;

public:
    Module *getParent();
    TypeKind *getReturnType();

    const std::vector<Argument*> &getArgs();
    Argument *getArg(unsigned i);

    BasicBlock *appendBasicBlock(const std::string &name = "");
    auto begin(); auto end();
    BasicBlock *getEntryBlock();

    void print(std::ostream &os) const;
};

#endif

#ifndef FUNC_H
#define FUNC_H

#include "IR/BasicBlock.hpp"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>

class Module;

class Func {
  private:
    std::string name;
    Module *parent;
    TypeKind *returnType;

    std::unordered_map<std::string, unsigned> nameMap;

    std::vector<Arg*> args;
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks;
  
  public: 
    Func(const std::string &name, Module *parent,
            TypeKind *retType, std::vector<Arg*> params);

    static Func *Create(const std::string &name, Module *parent,
            TypeKind *retType, std::vector<Arg*> params);

    std::string getName();
    std::string getUniqueName(const std::string &name);
    
    Module *getParent();
    TypeKind *getReturnType();

    std::vector<Arg*> &getArgs();
    Arg *getArg(unsigned i);

    BasicBlock *appendBasicBlock(const std::string &name);
    BasicBlock *appendBasicBlock(std::unique_ptr<BasicBlock> bb);

    BasicBlock *getEntryBlock();

    void print(std::ostream &os);
};

#endif

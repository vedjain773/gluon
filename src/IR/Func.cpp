#include "IR/Func.hpp"
#include "IR/Module.hpp"

Func::Func(const std::string &name, Module *parent,
        TypeKind *retType, std::vector<TypeKind*> paramTypes)
    :name(name), parent(parent), returnType(retType), paramTypes(paramTypes) {}

Module *Func::getParent() { return parent; }

TypeKind *Func::getReturnType() { return returnType; }

std::string Func::getName() { return name; }

std::vector<Arg*> &Func::getArgs() { return args; }

Arg *Func::getArg(unsigned i) { return args[i]; }

BasicBlock *Func::appendBasicBlock(const std::string &name) {
    auto bb = std::make_unique<BasicBlock>(name, this);
    basicBlocks.push_back(std::move(bb));
    return basicBlocks.back().get();
}

BasicBlock *Func::appendBasicBlock(std::unique_ptr<BasicBlock> bb) {
    basicBlocks.push_back(std::move(bb));
    return basicBlocks.back().get();
}

BasicBlock *Func::getEntryBlock() { return basicBlocks.front().get(); };

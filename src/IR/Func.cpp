#include "IR/Func.hpp"
#include "IR/Module.hpp"
#include <format>

Func::Func(const std::string &name, Module *parent, TypeKind *retType, std::vector<Arg*> args)
    :name(name), parent(parent), returnType(retType), args(args) {}

Func *Func::Create(const std::string &name, Module *parent,
        TypeKind *retType, std::vector<Arg*> args) 
{
    auto func = std::make_unique<Func>(name, parent, retType, args);
    Func *funcRaw = func.get();
    
    parent->appendFunc(std::move(func));
    return funcRaw;
}

Module *Func::getParent() { return parent; }

TypeKind *Func::getReturnType() { return returnType; }

std::string Func::getName() { return name; }

std::string Func::getUniqueName(const std::string &name) {
    if (nameMap.count(name) == 0) {
        nameMap[name] = 0;
    } else {
        nameMap[name] += 1;
    }

    return std::format("{}.{}", name, nameMap[name]);
}

std::vector<Arg*> &Func::getArgs() { return args; }

void Func::setArgs(std::vector<Arg*> args) {
    this->args = args; 
}

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

void Func::insertAtEnd(std::unique_ptr<BasicBlock> bb) {
    bb->setParent(this);
    basicBlocks.push_back(std::move(bb));
}

BasicBlock *Func::getEntryBlock() { return basicBlocks.front().get(); };

void Func::print(std::ostream &os) {
    os << std::format("define @{} (", name);

    for (auto &arg: args) {
        arg->print(os);
        
        if (arg != args.back())
            os << ", ";
    }

    os << std::format(") -> {}\n", getReturnType()->name);

    for (auto &bb: basicBlocks) {
        bb->print(os);
        os << '\n';
    }
}

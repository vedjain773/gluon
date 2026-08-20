#include "IR/BasicBlock.hpp"
#include "IR/Func.hpp"
#include <format>

BasicBlock::BasicBlock(const std::string &name, Func *parent)
    :name(name), parent(parent) {}

BasicBlock *BasicBlock::Create(const std::string &name, Func *parent) {
    auto bb = std::make_unique<BasicBlock>(name, parent);
    BasicBlock *bbRaw = bb.get();

    if (parent != nullptr) {
        parent->appendBasicBlock(std::move(bb));
    }

    return bbRaw;
}

std::unique_ptr<BasicBlock> BasicBlock::CreateDetached(const std::string &name) {
    auto bb = std::make_unique<BasicBlock>(name, nullptr);
    return bb;
}

Func *BasicBlock::getParent() { return parent; }

void BasicBlock::setParent(Func *parent) {
    this->parent = parent;
}

std::string BasicBlock::getName() { return name; }

void BasicBlock::appendInst(std::unique_ptr<Inst> inst) {
    instructions.push_back(std::move(inst));    
}

void BasicBlock::insertInst(Inst *before, std::unique_ptr<Inst> inst) {
    int i = 0;
    for (i = 0; i < instructions.size(); i++) {
        if (instructions[i].get() == before) 
            break;
    }

    instructions.insert(instructions.begin() + i, std::move(inst));
}

void BasicBlock::removeInst(Inst *inst) {
    int i = 0;
    for (i = 0; i < instructions.size(); i++) {
        if (instructions[i].get() == inst) 
            break;
    }

    instructions.erase(instructions.begin() + i);
}

Inst *BasicBlock::getFirstInst() {
    if (instructions.size())
        return instructions.front().get(); 
    else 
        return nullptr;
}

Inst *BasicBlock::getLastInst() { 
    if (instructions.size())
        return instructions.back().get();
    else 
        return nullptr;
}

Inst *BasicBlock::getTerminator() { 
    if (hasTerminator())
        return getLastInst();
    else
        return nullptr;
}
    
bool BasicBlock::hasTerminator() { 
    if (getLastInst() == nullptr)
        return false;
    else 
        return getLastInst()->isTerminator();
}

std::vector<BasicBlock*> &BasicBlock::getPredecessors() { return predecessors; }

std::vector<BasicBlock*> &BasicBlock::getSuccessors() { return successors; }

void BasicBlock::addPredecessor(BasicBlock *bb) { predecessors.push_back(bb); }

void BasicBlock::print(std::ostream &os) {
    os << std::format("{}: \n", name);

    for (auto &inst: instructions) {
        os << "  ";
        inst->print(os);
        os << '\n';
    }
}

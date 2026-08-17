#include "IR/BasicBlock.hpp"
#include "IR/Func.hpp"

BasicBlock::BasicBlock(const std::string &name, Func *parent)
    :name(name), parent(parent) {}

Func *BasicBlock::getParent() { return parent; }

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

Inst *BasicBlock::getFirstInst() { return instructions.front().get(); }

Inst *BasicBlock::getLastInst() { return instructions.back().get(); }

Inst *BasicBlock::getTerminator() { return getLastInst(); }
    
bool BasicBlock::hasTerminator() { return getLastInst()->isTerminator(); }

std::vector<BasicBlock*> &BasicBlock::getPredecessors() { return predecessors; }

std::vector<BasicBlock*> &BasicBlock::getSuccessors() { return successors; }

void BasicBlock::addPredecessor(BasicBlock *bb) { predecessors.push_back(bb); }

#include "backend/mBlock.hpp"

using namespace RISCV;

mBlock::mBlock(BasicBlock &bb, mFunc *parent) :
    name(bb.getName()),
    parent(parent) {}

mBlock::mBlock(const std::string &blockName, mFunc *parent) :
    name(blockName),
    parent(parent) {}

std::string mBlock::getName() {
    return name;
}

mFunc *mBlock::getParent() {
    return parent;
}

void mBlock::appendInst(std::unique_ptr<mInst> inst) {
    instructions.push_back(std::move(inst));
}

void mBlock::appendAtTop(std::unique_ptr<mInst> inst) {
    instructions.insert(instructions.begin(), std::move(inst));
}

void mBlock::insertBefore(std::unique_ptr<mInst> inst, mInst *before) {
    unsigned i, numInsts = instructions.size();
    for (i = 0; i < numInsts; i++) {
        if (instructions[i].get() == before) break;
    }

    instructions.insert(instructions.begin() + i, std::move(inst)); 
}

void mBlock::insertAfter(std::unique_ptr<mInst> inst, mInst *after) {
    unsigned i, numInsts = instructions.size();
    for (i = 0; i < numInsts; i++) {
        if (instructions[i].get() == after) break;
    }

    instructions.insert(instructions.begin() + i + 1, std::move(inst)); 
}

void mBlock::insertPnel(std::unique_ptr<mInst> inst) {
    instructions.insert(instructions.end() - 1, std::move(inst));
}

void mBlock::removeInst(mInst *inst) {
    unsigned i;

    for (i = 0; i < instructions.size(); i++) {
        mInst *curr = instructions[i].get();
        
        if (curr == inst) break;
    }

    instructions.erase(instructions.begin() + i);
}

std::vector<std::unique_ptr<mInst>> &mBlock::getInsts() {
    return instructions;
}

unsigned mBlock::getNumInsts() {
    return instructions.size();
}

mInst *mBlock::getTerminator() {
    return instructions.back().get();
}

std::vector<mBlock *> &mBlock::getPreds() {
    return preds;
}

std::vector<mBlock *> &mBlock::getSuccs() {
    return succs;
}

void mBlock::addPred(mBlock *bb) {
    preds.push_back(bb);
    bb->addSucc(this);
}

void mBlock::addSucc(mBlock *bb) {
    succs.push_back(bb);
}

void mBlock::remRet() {
    if (!instructions.empty() && instructions.back()->getOpCode() == Code::RET)
        instructions.pop_back();
}

void mBlock::print(std::ostream &os) {
    os << std::format(".L{}:\n", name);

    for (auto &inst : instructions) {
        os << " ";
        inst->print(os);
        os << "\n";
    }

    os << "\n";
}

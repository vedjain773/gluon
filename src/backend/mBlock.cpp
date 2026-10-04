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

std::vector<std::unique_ptr<mInst>> &mBlock::getInsts() {
    return instructions;
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

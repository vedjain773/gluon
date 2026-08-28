#include "backend/riscv64/mBlock.hpp"

using namespace RISCV;

mBlock::mBlock(BasicBlock &bb, mFunc *parent)
    :name(bb.getName()), parent(parent) {}

std::string mBlock::getName() { return name; }

mFunc *mBlock::getParent() { return parent; }

void mBlock::appendInst(std::unique_ptr<mInst> inst) {
    instructions.push_back(std::move(inst));
}

std::vector<mBlock*> &mBlock::getPreds() { return preds; }

std::vector<mBlock*> &mBlock::getSuccs() { return succs; }

void mBlock::addPred(mBlock *bb) { preds.push_back(bb); }

void mBlock::print(std::ostream &os) {
    os << std::format("{}:\n", name);

    for (auto &inst: instructions) {
        inst->print(os);
        os << '\n';
    }
}

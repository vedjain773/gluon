#include "backend/riscv64/mFunc.hpp"

using namespace RISCV;

mFunc::mFunc(Func &func, mModule *parent)
    :name(func.getName()), parent(parent) {}

std::string mFunc::getName() { return name; }

mModule *mFunc::getParent() { return parent; }

mBlock *mFunc::appendBlock(std::unique_ptr<mBlock> bb) {
    blocks.push_back(std::move(bb));

    return blocks.back().get();
}

std::vector<std::unique_ptr<mBlock>> &mFunc::getBlocks() {
    return blocks;
}

mBlock *mFunc::getEntryBlock() { return blocks[0].get(); }

void mFunc::print(std::ostream &os) {
    os << std::format("func {} \n", name);

    for (auto &bb: blocks) bb->print(os);
}

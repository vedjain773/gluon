#include "backend/riscv64/mFunc.hpp"

using namespace RISCV;

mFunc::mFunc(Func &func, mModule *parent)
    :name(func.getName()), parent(parent) {}

std::string mFunc::getName() { return name; }

mModule *mFunc::getParent() { return parent; }

mBlock *mFunc::appendBlock(std::unique_ptr<mBlock> bb) {
    blocks.push_back(std::move(bb));
}

mBlock *mFunc::getEntryBlock() { return blocks[0].get(); }

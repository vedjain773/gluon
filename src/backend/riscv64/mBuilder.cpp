#include "backend/riscv64/mBuilder.hpp"

using namespace RISCV;

mBuilder::mBuilder(mModule *currModule)
    :currModule(currModule) {}

void mBuilder::setInsertPoint(mBlock *block) {
    currBlock = block;
    currFunc = currBlock->getParent();
    currModule = currFunc->getParent();
}

mBlock *mBuilder::getInsertPoint() { return currBlock; }

mInst *mBuilder::createInst(OpCode opcode, std::vector<mOperand> operands) {
    auto minst = std::make_unique<mInst>(opcode, currBlock, operands);

    auto *minstRaw = minst.get();
    currBlock->appendInst(std::move(minst));

    return minstRaw;
}

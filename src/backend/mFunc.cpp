#include "backend/mFunc.hpp"

using namespace RISCV;

mFunc::mFunc(Func &func, mModule *parent) :
    name(func.getName()),
    parent(parent) {}

std::string mFunc::getName() {
    return name;
}

mModule *mFunc::getParent() {
    return parent;
}

unsigned mFunc::getNextSlotId() {
    unsigned id = stackSlots.empty() ? 0 : stackSlots.back()->getSlotId() + 1;
    return id;
}

std::vector<StackSlot *> &mFunc::getStackSlots() {
    return stackSlots;
}

void mFunc::insertSlot(StackSlot *slot) {
    stackSlots.push_back(slot);
}

void mFunc::setFrameSize(unsigned size) {
    frameSize = size;
}

void mFunc::addPrologue() {
    mBlock *entry = blocks.front().get();
    TypeKind *intType = getType("int");

    PhyReg *sp = PhyReg::Create(Reg::SP);
    PhyReg *ra = PhyReg::Create(Reg::RA);
    PhyReg *s0 = PhyReg::Create(Reg::S0);

    std::vector<mOperand *> stackOffsetOpers = {sp, sp, Immediate::Create(-frameSize, intType)};
    auto stackOffset = std::make_unique<mInst>(Code::ADDI, entry, stackOffsetOpers);

    std::vector<mOperand *> storeRAOpers = {ra,
                                            MemOperand::Create(intType, Reg::SP, frameSize - 8)};
    auto storeRA = std::make_unique<mInst>(Code::SD, entry, storeRAOpers);

    std::vector<mOperand *> storeS0Opers = {s0,
                                            MemOperand::Create(intType, Reg::SP, frameSize - 16)};
    auto storeS0 = std::make_unique<mInst>(Code::SD, entry, storeS0Opers);

    std::vector<mOperand *> s0OffsetOpers = {s0, sp, Immediate::Create(frameSize, intType)};
    auto s0Offset = std::make_unique<mInst>(Code::ADDI, entry, s0OffsetOpers);

    entry->appendAtTop(std::move(s0Offset));
    entry->appendAtTop(std::move(storeS0));
    entry->appendAtTop(std::move(storeRA));
    entry->appendAtTop(std::move(stackOffset));
}

void mFunc::addEpilogue() {
    mBlock *end = blocks.back().get();
    end->remRet();

    TypeKind *intType = getType("int");

    PhyReg *sp = PhyReg::Create(Reg::SP);
    PhyReg *ra = PhyReg::Create(Reg::RA);
    PhyReg *s0 = PhyReg::Create(Reg::S0);

    std::vector<mOperand *> stackOffsetOpers = {sp, sp, Immediate::Create(frameSize, intType)};
    auto stackOffset = std::make_unique<mInst>(Code::ADDI, end, stackOffsetOpers);

    std::vector<mOperand *> loadRAOpers = {ra, MemOperand::Create(intType, Reg::SP, frameSize - 8)};
    auto loadRA = std::make_unique<mInst>(Code::LD, end, loadRAOpers);

    std::vector<mOperand *> loadS0Opers = {s0,
                                           MemOperand::Create(intType, Reg::SP, frameSize - 16)};
    auto loadS0 = std::make_unique<mInst>(Code::LD, end, loadS0Opers);

    end->appendInst(std::move(loadRA));
    end->appendInst(std::move(loadS0));
    end->appendInst(std::move(stackOffset));
    end->appendInst(std::make_unique<mInst>(Code::RET, end));
}

mBlock *mFunc::appendBlock(std::unique_ptr<mBlock> bb) {
    blocks.push_back(std::move(bb));

    return blocks.back().get();
}

std::vector<std::unique_ptr<mBlock>> &mFunc::getBlocks() {
    return blocks;
}

mBlock *mFunc::getEntryBlock() {
    return blocks[0].get();
}

void mFunc::print(std::ostream &os) {
    os << std::format("func {} \n", name);

    for (auto &bb : blocks)
        bb->print(os);
}

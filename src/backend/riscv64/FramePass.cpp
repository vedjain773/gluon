#include "backend/riscv64/FramePass.hpp"
#include <vector>

using namespace RISCV;
using size_t = std::size_t;

FramePass::FramePass(mModule *mmod) :
    mmod(mmod) {}

void FramePass::calcOffsets(mFunc *mfunc) {
    int offset = 16;
    offset += mfunc->getCSRsize() * 8;

    std::vector<StackSlot *> &ss = mfunc->getStackSlots();

    for (auto &slot : ss) {
        TypeKind *sType = slot->getType();

        if (offset % sType->align != 0) {
            int padding = sType->size - (offset % sType->align);
            offset += padding;
        }

        offset += sType->size;
        slot->setOffset(-offset);
    }

    if (offset % 16 != 0) {
        int padding = 16 - (offset % 16);
        offset += padding;
    }

    mfunc->setFrameSize(offset);
}

void FramePass::handlePLAmem(mInst *minst) {
    MemOperand *mem = dynamic_cast<MemOperand*>(minst->getOperand(1));
    Reg reg = mem->getBase();
    int offset = mem->getOffset();

    minst->setOpCode(Code::ADDI);
    minst->setOperand(1, PhyReg::Create(reg));
    minst->setOperand(2, Immediate::Create(offset, getType("int")));
}

void FramePass::expand(mFunc *mfunc) {
    for (auto &mblock: mfunc->getBlocks()) {
        for (auto &minst: mblock->getInsts()) {
            Code code = minst->getOpCode();

            auto isLoadStore = [](Code code) {
                switch (code) {
                    case Code::LB:
                    case Code::LH:
                    case Code::LA:
                    case Code::LW:
                    case Code::LD: return true;

                    case Code::SB:
                    case Code::SH:
                    case Code::SW:
                    case Code::SD: return true;

                    default: return false;
                }
            };

            if (isLoadStore(code) && minst->getOperand(1)->getOpkind() != OpKind::MemOperand) {
                //assuming operand has already been coloured
                PhyReg *reg = dynamic_cast<PhyReg*>(minst->getOperand(1));
                minst->setOperand(1, MemOperand::Create(getType("int"), reg->getReg(), 0));
                continue;
            }

            mInst *minstRaw = minst.get();

            if (code != Code::P_LA) continue;
            if (minst->getOperand(1)->getOpkind() != OpKind::MemOperand) continue; 
            handlePLAmem(minstRaw);
        }
    }
}

void FramePass::run() {
    for (auto &func : mmod->getFuncs()) {
        calcOffsets(func.get());
        expand(func.get());

        func->addPrologue();
        func->addEpilogue();
    }
}

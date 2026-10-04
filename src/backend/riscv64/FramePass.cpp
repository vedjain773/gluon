#include "backend/riscv64/FramePass.hpp"
#include <vector>

using namespace RISCV;
using size_t = std::size_t;

FramePass::FramePass(mModule *mmod)
    :mmod(mmod) {}

void FramePass::calcOffsets(mFunc *mfunc) {
    int offset = 16;  
    std::vector<StackSlot*> &ss = mfunc->getStackSlots();

    for (auto &slot: ss) {
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

void FramePass::run() {
    for (auto &func: mmod->getFuncs()) {
        calcOffsets(func.get());

        func->addPrologue();
        func->addEpilogue();
    }
}

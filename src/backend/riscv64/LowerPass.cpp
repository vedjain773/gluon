#include "backend/riscv64/LowerPass.hpp"

using namespace RISCV;

LowerPass::LowerPass(Module *module)
    :module(module), mmod(mModule::Create(*module)) {}

mModule *LowerPass::getModule() { return mmod; }

void LowerPass::lower() {
    for (auto &func: module->getFuncs()) lowerFunc(func.get());
}

void LowerPass::lowerFunc(Func *func) {
    auto mfunc = std::make_unique<mFunc>(*func, mmod);
    currFunc = mfunc.get();
    
    mmod->appendFunc(std::move(mfunc));

    for (auto &bb: func->getBlocks()) lowerBlock(bb.get());
}

void LowerPass::lowerBlock(BasicBlock *bb) {
    auto mblock = std::make_unique<mBlock>(*bb, currFunc);
    currBlock = mblock.get();

    currFunc->appendBlock(std::move(mblock));

    for (auto &inst: bb->getInsts()) lowerInst(inst.get());
}

void LowerPass::lowerInst(Inst *inst) {
    switch (inst->getOpCode()) {
        case OpCode::RET: handleRet(inst->getOperand(0));
        break;

        default: return;
    }
}

//---

void LowerPass::handleRet(Value *value) {
    if (value == nullptr) return;

    if (value->getValueKind() == ValueKind::Constant) {
        ConstantInt *cint = dynamic_cast<ConstantInt*>(value);
        uint64_t value = cint->getValue();
        
        std::vector<mOperand*> opers = {PhyReg::Create(0), Immediate::Create(value)};
        auto liInst = std::make_unique<mInst>(Code::LI, currBlock, opers);

        auto retInst = std::make_unique<mInst>(Code::RET, currBlock);
        currBlock->appendInst(std::move(liInst));
        currBlock->appendInst(std::move(retInst));
    } 
}

//---

void LowerPass::print(std::ostream &os) {
    mmod->print(os);
}

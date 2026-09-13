#include "backend/riscv64/LowerPass.hpp"

using namespace RISCV;

LowerPass::LowerPass(Module *module)
    :module(module), mmod(mModule::Create(*module)), currentRegNo(0) {}

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
        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::REM: handleBinOp(inst, inst->getOpCode());
        break;

        case OpCode::RET: handleRet(inst);
        break;

        default: return;
    }
}

//---

mOperand *LowerPass::materialize(mOperand *oper) {
    if (oper->getOpkind() == OpKind::Immediate) {
        mOperand *virtReg = VirtReg::Create(currentRegNo++);
        std::vector<mOperand*> opersL = {virtReg, oper};
        auto liInst = std::make_unique<mInst>(Code::LI, currBlock, opersL);

        currBlock->appendInst(std::move(liInst));
        return virtReg;
    }

    return oper;
}

//---

mOperand *LowerPass::insertReg(Value *value) {
    if (virtualRegTable.count(value)) return virtualRegTable[value];

    virtualRegTable.insert({value, VirtReg::Create(currentRegNo++)});
    return virtualRegTable[value];
}

//---

mOperand *LowerPass::handleValue(Value *value) {
    switch (value->getValueKind()) {
        case ValueKind::Constant: {
            ConstantInt *cint = dynamic_cast<ConstantInt*>(value);
            uint64_t value = cint->getValue();

            return Immediate::Create(value);
        } break;

        case ValueKind::Instruction: {
            return insertReg(value); 
        } break;

        default: return nullptr;
    } 
}

//---

void LowerPass::handleRet(Inst *inst) {
    Value *value = inst->getOperand(0);
    if (value == nullptr) return;

    mOperand *operand = handleValue(value); 
    std::vector<mOperand*> opers = {PhyReg::Create(Reg::A0), operand};

    Code opc = operand->getOpkind() == OpKind::Immediate ? Code::LI : Code::MV;
    auto liInst = std::make_unique<mInst>(opc, currBlock, opers);
    currBlock->appendInst(std::move(liInst));

    auto retInst = std::make_unique<mInst>(Code::RET, currBlock);
    currBlock->appendInst(std::move(retInst));
}

//---

void LowerPass::handleBinOp(Inst *inst, const OpCode &code) {
    Value *lhs = inst->getOperand(0);
    Value *rhs = inst->getOperand(1);

    if (lhs == nullptr || rhs == nullptr) return;

    mOperand *lhsVirtReg = materialize(handleValue(lhs));
    mOperand *rhsVirtReg = materialize(handleValue(rhs));

    mOperand *result = insertReg(inst);

    std::vector<mOperand*> opersAdd = {result, lhsVirtReg, rhsVirtReg};
    auto addInst = std::make_unique<mInst>(getCode(code), currBlock, opersAdd);

    currBlock->appendInst(std::move(addInst));
}

//---

Code RISCV::getCode(const OpCode &opc) {
    switch (opc) {
        case OpCode::ADD: return Code::ADD;
        case OpCode::SUB: return Code::SUB;
        case OpCode::MUL: return Code::MUL;
        case OpCode::DIV: return Code::DIV;
        case OpCode::REM: return Code::REM;

        default: return Code::NOP;
    }
}

//---

void LowerPass::print(std::ostream &os) {
    mmod->print(os);
}

#include "backend/riscv64/LowerPass.hpp"
#include <iostream>

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

    for (auto &bb: func->getBlocks()) {
        auto mblock = std::make_unique<mBlock>(*bb, currFunc);
        blockMap.insert({bb.get(), mblock.get()});
        currFunc->appendBlock(std::move(mblock));
    }

    for (auto &bb: func->getBlocks()) lowerBlock(bb.get());
}

void LowerPass::lowerBlock(BasicBlock *bb) {
    currBlock = blockMap[bb];

    for (auto &inst: bb->getInsts()) lowerInst(inst.get());
}

void LowerPass::lowerInst(Inst *inst) {
    switch (inst->getOpCode()) {
        case OpCode::NOT:
        case OpCode::NEG: handleUnaryOp(inst, inst->getOpCode());
        break;

        case OpCode::ADD:
        case OpCode::SUB:
        case OpCode::MUL:
        case OpCode::DIV:
        case OpCode::REM: handleBinOp(inst, inst->getOpCode());
        break; 

        case OpCode::LT:
        case OpCode::GT: 
        case OpCode::LTE:
        case OpCode::GTE:
        case OpCode::EQ:
        case OpCode::NEQ: handleCmpOp(inst, inst->getOpCode());
        break;

        case OpCode::ALLOCA: handleAlloca(inst); 
        break;

        case OpCode::LOAD: handleLoad(inst);
        break;

        case OpCode::STORE: handleStore(inst);
        break;

        case OpCode::BR: handleUBr(inst);
        break;

        case OpCode::BRC: handleCBr(inst);
        break;

        case OpCode::RET: handleRet(inst);
        break;

        default: return;
    }
}

//---

mOperand *LowerPass::materialize(mOperand *oper) {
    OpKind opkind = oper->getOpkind();

    if (opkind == OpKind::Immediate || opkind == OpKind::StackSlot) {
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

    VirtReg *virtReg = VirtReg::Create(currentRegNo++);
    virtualRegTable.insert({value, virtReg});
   
    return virtReg;
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

void LowerPass::handleUnaryOp(Inst *inst, const OpCode &code) {
    UnaryInst *uinst = dynamic_cast<UnaryInst*>(inst);

    mOperand *oper = materialize(handleValue(uinst->getUnaryOper()));
    mOperand *result = insertReg(inst);

    std::vector<mOperand*> opers = {result, oper};
    Code opc = code == OpCode::NEG ? Code::NEG : Code::SEQZ;

    auto ninst = std::make_unique<mInst>(opc, currBlock, opers);
    currBlock->appendInst(std::move(ninst));
}

//---

void LowerPass::handleBinOp(Inst *inst, const OpCode &code) {
    Value *lhs = inst->getOperand(0);
    Value *rhs = inst->getOperand(1);

    if (lhs == nullptr || rhs == nullptr) return;

    mOperand *lhsVirtReg = materialize(handleValue(lhs));
    mOperand *rhsVirtReg = materialize(handleValue(rhs));

    mOperand *result = insertReg(inst);

    std::vector<mOperand*> opers = {result, lhsVirtReg, rhsVirtReg};
    auto binInst = std::make_unique<mInst>(getCode(code), currBlock, opers);

    currBlock->appendInst(std::move(binInst));
}

//---

void LowerPass::handleCmpOp(Inst *inst, const OpCode &code) {
    Value *lhs = inst->getOperand(0);
    Value *rhs = inst->getOperand(1);

    if (lhs == nullptr || rhs == nullptr) return;

    mOperand *lhsVirtReg = materialize(handleValue(lhs));
    mOperand *rhsVirtReg = materialize(handleValue(rhs));

    mOperand *result = insertReg(inst);

    std::vector<mOperand*> opers = {result, lhsVirtReg, rhsVirtReg};
    std::vector<mOperand*> checkOpers = {result, result};

    Code codes[2];

    switch (code) {
        case OpCode::LT: 
        case OpCode::GT: codes[0] = getCode(code);
        break;

        case OpCode::LTE: 
        case OpCode::GTE: {
            codes[0] = code == OpCode::LTE ? Code::SGT : Code::SLT;
            codes[1] = Code::SEQZ;
        } break;

        case OpCode::EQ:
        case OpCode::NEQ: {
            codes[0] = Code::SUB;
            codes[1] = code == OpCode::EQ ? Code::SEQZ : Code::SNEZ;
        } break;

        default: {}
    }

    auto finst = std::make_unique<mInst>(codes[0], currBlock, opers);
    currBlock->appendInst(std::move(finst));

    if (code == OpCode::LT || code == OpCode::GT) return;

    auto check = std::make_unique<mInst>(codes[1], currBlock, checkOpers);
    currBlock->appendInst(std::move(check));
} 

//---

void LowerPass::handleAlloca(Inst *inst) {
    AllocaInst *ainst = dynamic_cast<AllocaInst*>(inst);

    TypeKind *type = ainst->getValType();
    unsigned id = currFunc->getNextSlotId();

    StackSlot *slot = StackSlot::Create(type->size, id);
    stackSlotTable.insert({inst, slot});
    currFunc->insertSlot(slot); 
}

//---

void LowerPass::handleLoad(Inst *inst) {
    LoadInst *loadInst = dynamic_cast<LoadInst*>(inst);
    insertReg(inst);

    mOperand *loadTo = virtualRegTable[inst];
    mOperand *loadFrom = stackSlotTable[loadInst->getValue()];

    Code opc = getLoadCode(loadFrom); 
    std::vector<mOperand*> opers = {loadTo, loadFrom};
    auto linst = std::make_unique<mInst>(opc, currBlock, opers);
   
    currBlock->appendInst(std::move(linst));
}

//---

void LowerPass::handleStore(Inst *inst) {
    StoreInst *storeInst = dynamic_cast<StoreInst*>(inst);

    mOperand *storeFrom = materialize(handleValue(storeInst->getValue()));
    mOperand *storeTo = stackSlotTable[storeInst->getDest()];

    Code opc = getStoreCode(storeTo);
    std::vector<mOperand*> opers = {storeFrom, storeTo};
    auto linst = std::make_unique<mInst>(opc, currBlock, opers);
    
    currBlock->appendInst(std::move(linst));
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

void LowerPass::handleUBr(Inst *inst) {
    UnCondBrInst *brinst = dynamic_cast<UnCondBrInst*>(inst);
    
    mBlock *jBlock = blockMap[brinst->getThenBlock()];

    auto jinst = std::make_unique<mBrInst>(Code::J, currBlock, nullptr, jBlock);
    currBlock->appendInst(std::move(jinst));
}

void LowerPass::handleCBr(Inst *inst) {
    CondBrInst *brinst = dynamic_cast<CondBrInst*>(inst);

    Value *condVal = brinst->getCond();
    
    mBlock *thenBlock = blockMap[brinst->getThenBlock()];
    mBlock *elseBlock = blockMap[brinst->getElseBlock()];

    mOperand *cond = materialize(handleValue(condVal));

    auto binst = std::make_unique<mBrInst>(Code::BNEZ, currBlock, cond, thenBlock);
    currBlock->appendInst(std::move(binst));

    auto jinst = std::make_unique<mBrInst>(Code::J, currBlock, nullptr, elseBlock);
    currBlock->appendInst(std::move(jinst));
}

//---

Code RISCV::getCode(const OpCode &opc) {
    switch (opc) {
        case OpCode::ADD: return Code::ADD;
        case OpCode::SUB: return Code::SUB;
        case OpCode::MUL: return Code::MUL;
        case OpCode::DIV: return Code::DIV;
        case OpCode::REM: return Code::REM;
        case OpCode::LT: return Code::SLT;
        case OpCode::GT: return Code::SGT;
        default: return Code::NOP;
    }
}

Code RISCV::getStoreCode(mOperand *oper) {
    StackSlot *ss = dynamic_cast<StackSlot*>(oper);
    if (ss == nullptr) return Code::SD;

    unsigned size = ss->getSlotSize();

    switch (size) {
        case 1: return Code::SB;
        case 2: return Code::SH;
        case 4: return Code::SW;
        default: return Code::SD;
    }
}

Code RISCV::getLoadCode(mOperand *oper) {
    StackSlot *ss = dynamic_cast<StackSlot*>(oper);
    if (ss == nullptr) return Code::LD;

    unsigned size = ss->getSlotSize();

    switch (size) {
        case 1: return Code::LB;
        case 2: return Code::LH;
        case 4: return Code::LW;
        default: return Code::LD;
    }
}

//---

void LowerPass::print(std::ostream &os) {
    mmod->print(os);
}

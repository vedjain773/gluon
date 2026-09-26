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

        case OpCode::GEP: handleGEP(inst);
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
        TypeKind *type = oper->getType();
        mOperand *virtReg = newVirtReg(type);
        emit(Code::LI, {virtReg, oper});

        return virtReg;
    }

    return oper;
}

//---

mOperand *LowerPass::insertReg(Value *value) {
    if (virtualRegTable.count(value)) return virtualRegTable[value];

    VirtReg *virtReg = newVirtReg(value->getType());
    virtualRegTable.insert({value, virtReg});
   
    return virtReg;
}

//---

mOperand *LowerPass::handleValue(Value *value) {
    switch (value->getValueKind()) {
        case ValueKind::Constant: {
            ConstantInt *cint = dynamic_cast<ConstantInt*>(value);
            uint64_t cvalue = cint->getValue();

            return Immediate::Create(cvalue, value->getType());
        } break;

        case ValueKind::Instruction: {
            return insertReg(value); 
        } break;

        default: return nullptr;
    } 
}

mOperand *LowerPass::handleAddr(Value *value) {
    if (stackSlotTable.count(value) != 0) return stackSlotTable[value];
    else if (virtualRegTable.count(value) != 0) return virtualRegTable[value];
    else return nullptr;
}

//---

void LowerPass::handleUnaryOp(Inst *inst, const OpCode &code) {
    UnaryInst *uinst = dynamic_cast<UnaryInst*>(inst);

    mOperand *oper = materialize(handleValue(uinst->getUnaryOper()));
    mOperand *result = insertReg(inst);

    Code opc = code == OpCode::NEG ? Code::NEG : Code::SEQZ;
    emit(opc, {result, oper});
}

//---

void LowerPass::handleBinOp(Inst *inst, const OpCode &code) {
    Value *lhs = inst->getOperand(0);
    Value *rhs = inst->getOperand(1);

    if (lhs == nullptr || rhs == nullptr) return;

    mOperand *lhsVirtReg = materialize(handleValue(lhs));
    mOperand *rhsVirtReg = materialize(handleValue(rhs));

    mOperand *result = insertReg(inst);
    emit(getCode(code), {result, lhsVirtReg, rhsVirtReg});
}

//---

void LowerPass::handleCmpOp(Inst *inst, const OpCode &code) {
    Value *lhs = inst->getOperand(0);
    Value *rhs = inst->getOperand(1);

    if (lhs == nullptr || rhs == nullptr) return;

    mOperand *lhsVirtReg = materialize(handleValue(lhs));
    mOperand *rhsVirtReg = materialize(handleValue(rhs));

    mOperand *subResult = newVirtReg(getType("int"));
    mOperand *result = insertReg(inst);

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

    emit(codes[0], {subResult, lhsVirtReg, rhsVirtReg});

    if (code == OpCode::LT || code == OpCode::GT) return;

    emit(codes[1], {result, subResult});
} 

//---

void LowerPass::handleAlloca(Inst *inst) {
    AllocaInst *ainst = dynamic_cast<AllocaInst*>(inst);

    TypeKind *type = ainst->getValType();
    unsigned id = currFunc->getNextSlotId();

    StackSlot *slot = StackSlot::Create(id, type);
    stackSlotTable.insert({inst, slot});
    currFunc->insertSlot(slot); 
}

//---

void LowerPass::handleGEP(Inst *inst) {
    GEPInst *gepInst = dynamic_cast<GEPInst*>(inst);
    TypeKind *intType = getType("int");

    Value *ptr = gepInst->getPointerOp();
    std::vector<Value*> indices = gepInst->getIndices(); 

    //Get base address
    mOperand *val = genAddr(ptr);

    //Calculate offset
    unsigned numIndices = gepInst->getNumIndices();
    TypeKind *currType = gepInst->getSrcType();
    mOperand *offset = PhyReg::Create(Reg::ZERO);

    for (int i = 0; i < numIndices; i++) {
        mOperand *index = materialize(handleValue(indices[i]));
        mOperand *sizeImm = Immediate::Create(currType->size, intType); 
        
        mOperand *mulres = genOpInst(Code::MUL, {index, sizeImm}, intType);
        mOperand *addres = genOpInst(Code::ADD, {offset, mulres}, intType);

        offset = addres;
        currType = currType->to;
    }  

    mOperand *newAddr = insertReg(inst);
    emit(Code::ADD, {newAddr, val, offset});
}

//---

void LowerPass::handleLoad(Inst *inst) {
    LoadInst *loadInst = dynamic_cast<LoadInst*>(inst);
    insertReg(inst);

    mOperand *loadTo = virtualRegTable[inst];
    mOperand *loadFrom = handleAddr(loadInst->getValue());

    Code opc = getLoadCode(loadTo);
    emit(opc, {loadTo, loadFrom});
} 

//---

void LowerPass::handleStore(Inst *inst) {
    StoreInst *storeInst = dynamic_cast<StoreInst*>(inst);
    Value *value = storeInst->getValue();
    Value *dest = storeInst->getDest();

    mOperand *storeVal = nullptr;

    if (isPointerType(value->getType())) storeVal = genAddr(value);
    else storeVal = materialize(handleValue(value));
    
    mOperand *storeTo = handleAddr(dest);

    Code opc = getStoreCode(storeVal);
    emit(opc, {storeVal, storeTo});
}

//---

void LowerPass::handleRet(Inst *inst) {
    Value *value = inst->getOperand(0);
    if (value == nullptr) return;

    mOperand *operand = handleValue(value); 
    Code opc = operand->getOpkind() == OpKind::Immediate ? Code::LI : Code::MV;

    emit(opc, {PhyReg::Create(Reg::A0), operand});
    emit(Code::RET, {});
}

//---

void LowerPass::handleUBr(Inst *inst) {
    UnCondBrInst *brinst = dynamic_cast<UnCondBrInst*>(inst);
    
    mBlock *jBlock = blockMap[brinst->getThenBlock()];

    emitBr(Code::J, nullptr, jBlock);
}

void LowerPass::handleCBr(Inst *inst) {
    CondBrInst *brinst = dynamic_cast<CondBrInst*>(inst);

    Value *condVal = brinst->getCond();
    
    mBlock *thenBlock = blockMap[brinst->getThenBlock()];
    mBlock *elseBlock = blockMap[brinst->getElseBlock()];

    mOperand *cond = materialize(handleValue(condVal));

    emitBr(Code::BNEZ, cond, thenBlock);
    emitBr(Code::J, nullptr, elseBlock);
}

//---

mOperand *LowerPass::newVirtReg(TypeKind *type) {
    return VirtReg::Create(currentRegNo++, type);
}

void LowerPass::emit(const Code &code, std::initializer_list<mOperand*> opers) {
    std::vector<mOperand*> opersVec(opers);

    auto inst = std::make_unique<mInst>(code, currBlock, opersVec);
    currBlock->appendInst(std::move(inst));
}

void LowerPass::emitBr(const Code &code, mOperand *oper, mBlock *block) {
    auto inst = std::make_unique<mBrInst>(code, currBlock, oper, block);
    currBlock->appendInst(std::move(inst)); 
}

mOperand *LowerPass::genOpInst(const Code &code, std::initializer_list<mOperand*> opers,
         TypeKind *type)
{
    mOperand *result = newVirtReg(type);
    std::vector<mOperand*> newOpers = {result};
    
    newOpers.insert(newOpers.end(), opers);
    auto inst = std::make_unique<mInst>(code, currBlock, newOpers);
    currBlock->appendInst(std::move(inst));

    return result;
}

mOperand *LowerPass::genAddr(Value *value) {
    mOperand *addr = handleAddr(value);
    mOperand *addrHolder = newVirtReg(value->getType());

    emit(Code::P_LA, {addrHolder, addr});
    return addrHolder;
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
    TypeKind *type = oper->getType(); 
    std::size_t size = type == nullptr ? 8 : type->size; 

    switch (size) {
        case 1: return Code::SB;
        case 2: return Code::SH;
        case 4: return Code::SW;
        default: return Code::SD;
    }
}

Code RISCV::getLoadCode(mOperand *oper) {
    TypeKind *type = oper->getType();
    std::size_t size = type == nullptr ? 8 : type->size; 

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

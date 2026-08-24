#include "IR/IRBuilder.hpp"

IRBuilder::IRBuilder(Module *module) { currModule = module; }

void IRBuilder::setInsertPoint(BasicBlock *bb) {
    currBasicBlock = bb;
    currFunc = bb->getParent();
    currModule = currFunc->getParent();
}

BasicBlock *IRBuilder::getInsertBlock() { return currBasicBlock; }

Value *IRBuilder::createBinOp(OpCode op, Value *lhs, Value *rhs, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<BinaryInst> inst(BinaryInst::Create(op, lhs, rhs, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

Value *IRBuilder::createCmp(OpCode pred, Value *lhs, Value *rhs, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<CompInst> inst(CompInst::Create(pred, lhs, rhs, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

Value *IRBuilder::createNeg(Value *value, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<UnaryInst> inst(UnaryInst::Create(OpCode::NEG, value, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

Value *IRBuilder::createNot(Value *value, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<UnaryInst> inst(UnaryInst::Create(OpCode::NOT, value, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

Value *IRBuilder::createZExt(Value *value, TypeKind *type, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<ZExtInst> inst(ZExtInst::Create(value, type, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

Value *IRBuilder::createGEP(TypeKind *type, Value *ptr, std::vector<Value*> idx,
        const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<GEPInst> inst(GEPInst::Create(type, ptr, idx, name));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

AllocaInst *IRBuilder::createAlloca(TypeKind *type, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<AllocaInst> inst(AllocaInst::Create(type, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

AllocaInst *IRBuilder::createEntryAlloca(TypeKind *type, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<AllocaInst> inst(AllocaInst::Create(type, nname));

    auto *instRaw = inst.get();
    BasicBlock *entry = currFunc->getEntryBlock();
    entry->insertInst(entry->getFirstInst(), std::move(inst));

    return instRaw;
}

LoadInst *IRBuilder::createLoad(TypeKind *type, Value *ptr, const std::string &name) {
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<LoadInst> inst(LoadInst::Create(type, ptr, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

StoreInst *IRBuilder::createStore(Value *value, Value *ptr) {
    std::unique_ptr<StoreInst> inst(StoreInst::Create(value, ptr));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

CallInst *IRBuilder::createCall(Func *callee, std::vector<Value*> args,
        const std::string &name)
{
    std::string nname = currFunc->getUniqueName(name);
    std::unique_ptr<CallInst> inst(CallInst::Create(callee, args, nname));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

ReturnInst *IRBuilder::createRet(Value *value) { 
    std::unique_ptr<ReturnInst> inst(ReturnInst::Create(value));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

UnCondBrInst *IRBuilder::createBr(BasicBlock *then) {
    then->addPredecessor(currBasicBlock);

    std::unique_ptr<UnCondBrInst> inst(UnCondBrInst::Create(then));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

CondBrInst *IRBuilder::createCondBr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB) {
    trueBB->addPredecessor(currBasicBlock);
    falseBB->addPredecessor(currBasicBlock);

    std::unique_ptr<CondBrInst> inst(CondBrInst::Create(cond, trueBB, falseBB));

    auto *instRaw = inst.get();
    currBasicBlock->appendInst(std::move(inst));

    return instRaw;
}

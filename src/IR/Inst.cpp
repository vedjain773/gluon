#include "IR/Inst.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Func.hpp"

Inst::Inst(OpCode op, TypeKind *type, std::vector<Value*> ops, const std::string &name)
    :Value(type, ValueKind::Instruction, 0, name), opcode(op), operands(ops) {}

OpCode Inst::getOpCode() { return opcode; }

BasicBlock *Inst::getParent() { return parent; }

void Inst::setParent(BasicBlock *bb) { parent = bb; }

unsigned Inst::getNumOperands() { return operands.size(); }

Value *Inst::getOperand(unsigned i) { return operands[i]; }

void Inst::setOperand(unsigned i, Value *v) { operands[i] = v; }

bool Inst::isTerminator() { 
    switch (opcode) {
        case OpCode::RET:
        case OpCode::BR:
        case OpCode::BRC: return true;
        break;

        default: return false;
    }
}

//---

UnaryInst::UnaryInst(OpCode opcode, Value *operand, const std::string &name)
    :Inst(opcode, operand->getType(), {operand}, name) {}

UnaryInst *UnaryInst::Create(OpCode opcode, Value* operand, const std::string &name) {
    return new UnaryInst(opcode, operand, name);
}

Value *UnaryInst::getUnaryOper() { return getOperand(0); }

//---

BinaryInst::BinaryInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name)
    :Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

BinaryInst *BinaryInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new BinaryInst(opcode, lhs, rhs, name);
}

Value *BinaryInst::getLHS() { return getOperand(0); }

Value *BinaryInst::getRHS() { return getOperand(1); }

//---

CompInst::CompInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name)
    :Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

CompInst *CompInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new CompInst(opcode, lhs, rhs, name);
}

Value *CompInst::getLHS() { return getOperand(0); }

Value *CompInst::getRHS() { return getOperand(1); }

//---

AllocaInst::AllocaInst(TypeKind *type, const std::string &name)
    :Inst(OpCode::ALLOCA, type, {}, name) {}

AllocaInst *AllocaInst::Create(TypeKind *type, const std::string &name) {
    return new AllocaInst(type, name);
}

//---

LoadInst::LoadInst(TypeKind *type, Value *value, const std::string &name)
    :Inst(OpCode::LOAD, type, {value}, name) {}

LoadInst *LoadInst::Create(TypeKind *type, Value *value, const std::string &name) {
    return new LoadInst(type, value, name);
} 

//---

StoreInst::StoreInst(Value* val, Value *dest)
    :Inst(OpCode::STORE, val->getType(), {val, dest}, "") {}

StoreInst *StoreInst::Create(Value *val, Value *dest) {
    return new StoreInst(val, dest);
} 

//---

CallInst::CallInst(Func *callee, std::vector<Value*> args, const std::string &name)
    :Inst(OpCode::CALL, callee->getReturnType(), args, name), callee(callee)
{
    
}

CallInst *CallInst::Create(Func *callee, std::vector<Value*> args, const std::string &name) {
    return new CallInst(callee, args, name);
}

//---

CondBrInst::CondBrInst(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB)
    :Inst(OpCode::BRC, nullptr, {condn}, ""), trueBB(trueBB), falseBB(falseBB) {}

CondBrInst *CondBrInst::Create(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB) {
    return new CondBrInst(condn, trueBB, falseBB);
}

Value *CondBrInst::getCond() { return getOperand(0); }

BasicBlock *CondBrInst::getThenBlock() { return trueBB; }

BasicBlock *CondBrInst::getElseBlock() { return falseBB; }

//---

UnCondBrInst::UnCondBrInst(BasicBlock *thenBB)
    :Inst(OpCode::BR, nullptr, {}, ""), then(thenBB) {}

UnCondBrInst *UnCondBrInst::Create(BasicBlock *thenBB) {
    return new UnCondBrInst(thenBB);
}

BasicBlock *UnCondBrInst::getThenBlock() { return then; }

//---

ReturnInst::ReturnInst(Value *value)
    :Inst(OpCode::RET, value->getType(), {value}, "") {}

ReturnInst *ReturnInst::Create(Value *value) { 
    return new ReturnInst(value);
} 

//---

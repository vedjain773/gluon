#include "IR/Inst.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Func.hpp"
#include <format>

//---

std::string opcodeToStr(OpCode opcode) {
    switch (opcode) {
        case OpCode::NOT: return "not";
        case OpCode::NEG: return "neg";
        case OpCode::ADD: return "add";
        case OpCode::SUB: return "sub";
        case OpCode::MUL: return "mul";
        case OpCode::DIV: return "div";
        case OpCode::REM: return "rem";
        case OpCode::GT: return "gt";
        case OpCode::GTE: return "gte";
        case OpCode::LT: return "lt";
        case OpCode::LTE: return "lte";
        case OpCode::EQ: return "eq";
        case OpCode::NEQ: return "neq";
        case OpCode::LAND: return "land";
        case OpCode::LOR: return "lor";
        case OpCode::ALLOCA: return "alloca";
        case OpCode::LOAD: return "load";
        case OpCode::STORE: return "store";
        case OpCode::RET: return "ret";
        case OpCode::BR: return "br";
        case OpCode::BRC: return "br";
        case OpCode::CALL: return "call";
        default: return "";
    }  
}

//---

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

UnaryInst::UnaryInst(OpCode opcode, Value *value, const std::string &name)
    :Inst(opcode, value->getType(), {value}, name) {}

UnaryInst *UnaryInst::Create(OpCode opcode, Value* value, const std::string &name) {
    return new UnaryInst(opcode, value, name);
}

Value *UnaryInst::getUnaryOper() { return getOperand(0); }

void UnaryInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()) );
    getUnaryOper()->printAsOperand(os);
}

//---

BinaryInst::BinaryInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name)
    :Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

BinaryInst *BinaryInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new BinaryInst(opcode, lhs, rhs, name);
}

Value *BinaryInst::getLHS() { return getOperand(0); }

Value *BinaryInst::getRHS() { return getOperand(1); }

void BinaryInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()) );
    getLHS()->printAsOperand(os);
    getRHS()->printAsOperand(os);
}

//---

CompInst::CompInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name)
    :Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

CompInst *CompInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new CompInst(opcode, lhs, rhs, name);
}

Value *CompInst::getLHS() { return getOperand(0); }

Value *CompInst::getRHS() { return getOperand(1); }

void CompInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()) );
    getLHS()->printAsOperand(os);
    getRHS()->printAsOperand(os);
}

//---

AllocaInst::AllocaInst(TypeKind *type, const std::string &name)
    :Inst(OpCode::ALLOCA, type, {}, name) {}

AllocaInst *AllocaInst::Create(TypeKind *type, const std::string &name) {
    return new AllocaInst(type, name);
}

void AllocaInst::print(std::ostream &os) {
    os << std::format("{} = {} {}", getName(), opcodeToStr(OpCode::ALLOCA), getType()->name); 
}

//---

LoadInst::LoadInst(TypeKind *type, Value *value, const std::string &name)
    :Inst(OpCode::LOAD, type, {value}, name) {}

LoadInst *LoadInst::Create(TypeKind *type, Value *value, const std::string &name) {
    return new LoadInst(type, value, name);
} 

void LoadInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(OpCode::LOAD));
    getOperand(0)->printAsOperand(os);
} 

//---

StoreInst::StoreInst(Value *value, Value *dest)
    :Inst(OpCode::STORE, value->getType(), {value, dest}, "") {}

StoreInst *StoreInst::Create(Value *value, Value *dest) {
    return new StoreInst(value, dest);
} 

void StoreInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::STORE));
    getOperand(0)->printAsOperand(os);
    getOperand(1)->printAsOperand(os);
}

//---

CallInst::CallInst(Func *callee, std::vector<Value*> args, const std::string &name)
    :Inst(OpCode::CALL, callee->getReturnType(), args, name), callee(callee)
{
    unsigned i = 0;
    for (auto &arg: args) {
        callArgs.push_back(Arg::Create(arg->getType(), arg->getName(), callee, i));
        i++;
    } 
}

CallInst *CallInst::Create(Func *callee, std::vector<Value*> args, const std::string &name) {
    return new CallInst(callee, args, name);
}

void CallInst::print(std::ostream &os) {
    os << std::format("call @{}", func->getName());

    for (auto &arg: callArgs) {
        os << arg->print(os);
        os << ", ";
    }
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

void CondBrInst::print(std::ostream &os) {
    os << std::format("{} {} {}", opcodeToStr(OpCode::BRC), 
            trueBB->getName(), falseBB->getName()); 
}

//---

UnCondBrInst::UnCondBrInst(BasicBlock *thenBB)
    :Inst(OpCode::BR, nullptr, {}, ""), then(thenBB) {}

UnCondBrInst *UnCondBrInst::Create(BasicBlock *thenBB) {
    return new UnCondBrInst(thenBB);
}

BasicBlock *UnCondBrInst::getThenBlock() { return then; }

void UnCondBrInst::print(std::ostream &os) {
    os << std::format("{} {}", opcodeToStr(OpCode::BR), then->getName());
}

//---

ReturnInst::ReturnInst(Value *value)
    :Inst(OpCode::RET, value->getType(), {value}, "") {}

ReturnInst *ReturnInst::Create(Value *value) { 
    return new ReturnInst(value);
} 

void ReturnInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::RET));
    getOperand(0)->printAsOperand(os);
}

//---

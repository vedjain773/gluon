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
        case OpCode::GEP: return "gep";
        case OpCode::ZEXT: return "zext";
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
    :Value(type, ValueKind::Instruction, name), opcode(op), operands(ops) {}

OpCode Inst::getOpCode() { return opcode; }

BasicBlock *Inst::getParent() { return parent; }

void Inst::setParent(BasicBlock *bb) { parent = bb; }

unsigned Inst::getNumOperands() { return operands.size(); }

Value *Inst::getOperand(unsigned i) { return operands[i]; }

void Inst::setOperand(unsigned i, Value *v) { operands[i] = v; }

std::vector<Value*> &Inst::getOperands() { return operands; }

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
    os << ", ";
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
    os << ", ";
    getRHS()->printAsOperand(os);
}

//---

GEPInst::GEPInst(TypeKind *type, Value *ptr, std::vector<Value*> idxList, const std::string &name)
    :Inst(OpCode::GEP, type, {ptr}, name), idxList(idxList) {}

GEPInst *GEPInst::Create(TypeKind *type, Value *ptr, std::vector<Value*> idxList,
        const std::string &name) 
{
    return new GEPInst(type, ptr, idxList, name);
}

void GEPInst::print(std::ostream &os) {
    os << std::format("{} = {} {}, ", getName(), opcodeToStr(OpCode::GEP), getType()->name);
    getOperand(0)->printAsOperand(os);
    os << ", ";

    for (auto &idx: idxList) {
        idx->printAsOperand(os);
    
        if (idx != idxList.back())
            os << ", ";
    }
}

//---
ZExtInst::ZExtInst(Value *value, TypeKind *type, const std::string &name)
    :Inst(OpCode::ZEXT, type, {value}, name) {}

ZExtInst *ZExtInst::Create(Value *value, TypeKind *type, const std::string &name) {
    return new ZExtInst(value, type, name); 
}

void ZExtInst::print(std::ostream &os) {
    os << std::format("{} = {} {} to {}", getName(), opcodeToStr(OpCode::ZEXT),
            getOperand(0)->getType()->name, getType()->name);
}

//---

AllocaInst::AllocaInst(TypeKind *type, const std::string &name)
    :Inst(OpCode::ALLOCA, getPtrTo(type->name), {}, name), valType(type) {}

AllocaInst *AllocaInst::Create(TypeKind *type, const std::string &name) {
    return new AllocaInst(type, name);
}

TypeKind *AllocaInst::getValType() { return valType; }

void AllocaInst::print(std::ostream &os) {
    os << std::format("{} = {} {}", getName(), opcodeToStr(OpCode::ALLOCA), valType->name); 
}

//---

LoadInst::LoadInst(TypeKind *type, Value *ptr, const std::string &name)
    :Inst(OpCode::LOAD, type, {ptr}, name) {}

LoadInst *LoadInst::Create(TypeKind *type, Value *ptr, const std::string &name) {
    return new LoadInst(type, ptr, name);
}

Value *LoadInst::getValue() { return getOperand(0); }

void LoadInst::print(std::ostream &os) {
    os << std::format("{} = {} {} <- ", getName(), opcodeToStr(OpCode::LOAD), getType()->name);
    getValue()->printAsOperand(os);
} 

//---

StoreInst::StoreInst(Value *value, Value *ptr)
    :Inst(OpCode::STORE, value->getType(), {value, ptr}, "") {}

StoreInst *StoreInst::Create(Value *value, Value *ptr) {
    return new StoreInst(value, ptr);
} 

Value *StoreInst::getValue() { return getOperand(0); }

Value *StoreInst::getDest() { return getOperand(1); }

void StoreInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::STORE));
    getValue()->printAsOperand(os);
    os << " -> ";
    getDest()->printAsOperand(os);
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
    os << std::format("{} = call @{}", getName(), callee->getName());
    os << '(';

    for (auto &argVal: getOperands()) {
        argVal->printAsOperand(os);

        if (argVal != getOperands().back())
            os << ", ";
    }

    os << ')';
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
    os << std::format("{} ", opcodeToStr(OpCode::BRC));
    getCond()->printAsOperand(os);
    os << std::format(", {}, {}", getThenBlock()->getName(), getElseBlock()->getName());
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

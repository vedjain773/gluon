#include "IR/Inst.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Func.hpp"
#include <array>
#include <format>

//---

constexpr std::array<std::string, 26> opcodeNames{
    "not", "neg",  "add", "sub", "mul",  "div",    "rem",  "gt",    "gte", "lt", "lte", "eq",
    "neq", "land", "lor", "gep", "zext", "trunc",  "alloca","load", "store", "ret", "br", "br",
    "phi", "call"};

std::string opcodeToStr(OpCode opcode) {
    return opcodeNames[static_cast<unsigned>(opcode)];
}

//---

Inst::Inst(OpCode op, TypeKind *type, std::vector<Value *> ops, const std::string &name) :
    Value(type, ValueKind::Instruction, name),
    opcode(op),
    operands(ops) {

    unsigned i = 0;
    for (auto &operand: ops) {
        operand->addUse(this, i);
        i++;
    }
}

OpCode Inst::getOpCode() {
    return opcode;
}

BasicBlock *Inst::getParent() {
    return parent;
}

void Inst::setParent(BasicBlock *bb) {
    parent = bb;
}

void Inst::eraseFromParent() {
    for (unsigned i = 0; i < operands.size(); ++i) {
        if (!operands[i]) continue;
        operands[i]->removeUse(this, i);
    }

    getParent()->removeInst(this); 
}

unsigned Inst::getNumOperands() {
    return operands.size();
}

Value *Inst::getOperand(unsigned i) {
    return operands[i];
}

void Inst::setOperand(unsigned i, Value *v) {
    operands[i] = v;
}

std::vector<Value *> &Inst::getOperands() {
    return operands;
}

void Inst::addOperand(Value *value) {
    unsigned i = getNumOperands();
    operands.push_back(value);
    value->addUse(this, i);
}

void Inst::remOperand(unsigned i) {
    getOperand(i)->removeUse(this, i);
    operands.erase(operands.begin() + i);
    
    unsigned numOps = getNumOperands();

    for (unsigned idx = i; idx < numOps; idx++) {
        Use *use = operands[i]->getUse(this, i + 1);
        use->setOpNo(i);
    }
}

bool Inst::isBinary() {
    unsigned opid = static_cast<unsigned>(opcode);

    return 2 <= opid && opid <= 14;
}

bool Inst::isTerminator() {
    switch (opcode) {
        case OpCode::RET:
        case OpCode::BR:
        case OpCode::BRC:
            return true;
            break;

        default:
            return false;
    }
}

//---

UnaryInst::UnaryInst(OpCode opcode, Value *value, const std::string &name) :
    Inst(opcode, value->getType(), {value}, name) {}

UnaryInst *UnaryInst::Create(OpCode opcode, Value *value, const std::string &name) {
    return new UnaryInst(opcode, value, name);
}

Value *UnaryInst::getUnaryOper() {
    return getOperand(0);
}

void UnaryInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()));
    getUnaryOper()->printAsOperand(os);
}

//---

BinaryInst::BinaryInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) :
    Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

BinaryInst *BinaryInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new BinaryInst(opcode, lhs, rhs, name);
}

Value *BinaryInst::getLHS() {
    return getOperand(0);
}

Value *BinaryInst::getRHS() {
    return getOperand(1);
}

void BinaryInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()));
    getLHS()->printAsOperand(os);
    os << ", ";
    getRHS()->printAsOperand(os);
}

//---

CompInst::CompInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) :
    Inst(opcode, lhs->getType(), {lhs, rhs}, name) {}

CompInst *CompInst::Create(OpCode opcode, Value *lhs, Value *rhs, const std::string &name) {
    return new CompInst(opcode, lhs, rhs, name);
}

Value *CompInst::getLHS() {
    return getOperand(0);
}

Value *CompInst::getRHS() {
    return getOperand(1);
}

void CompInst::print(std::ostream &os) {
    os << std::format("{} = {} ", getName(), opcodeToStr(getOpCode()));
    getLHS()->printAsOperand(os);
    os << ", ";
    getRHS()->printAsOperand(os);
}

//---

GEPInst::GEPInst(TypeKind *type, Value *ptr, std::vector<Value *> idxList,
                 const std::string &name) :
    Inst(OpCode::GEP, getResultType(idxList.size(), ptr->getType()), getOps(ptr, idxList), name),
    srcType(type) {}

TypeKind *GEPInst::getResultType(unsigned size, TypeKind *ptr) {
    unsigned num = size;
    TypeKind *currType = ptr;

    while (num-- > 0)
        currType = currType->to;
    return getPtrTo(currType->name);
}

std::vector<Value *> GEPInst::getOps(Value *ptr, std::vector<Value *> idxList) {
    std::vector<Value *> ops = {ptr};
    ops.insert(ops.begin() + 1, idxList.begin(), idxList.end());

    return ops;
}

GEPInst *GEPInst::Create(TypeKind *type, Value *ptr, std::vector<Value *> idxList,
                         const std::string &name) {
    return new GEPInst(type, ptr, idxList, name);
}

TypeKind *GEPInst::getSrcType() {
    return srcType;
}

Value *GEPInst::getPointerOp() {
    return getOperand(0);
}

unsigned GEPInst::getNumIndices() {
   return getNumOperands() - 1;
}

std::vector<Value *> GEPInst::getIndices() {
    std::vector<Value *> idxList;
    idxList.reserve(getNumIndices());

    for (int i = 1; i < getNumOperands(); i++)
        idxList.push_back(getOperand(i));

    return idxList;
}

void GEPInst::print(std::ostream &os) {
    os << std::format("{} = {} {}, ", getName(), opcodeToStr(OpCode::GEP), srcType->name);
    std::vector<Value *> &opList = getOperands();

    for (auto &oper : opList) {
        oper->printAsOperand(os);

        if (oper != opList.back()) os << ", ";
    }
}

//---

ZExtInst::ZExtInst(Value *value, TypeKind *type, const std::string &name) :
    Inst(OpCode::ZEXT, type, {value}, name) {}

ZExtInst *ZExtInst::Create(Value *value, TypeKind *type, const std::string &name) {
    return new ZExtInst(value, type, name);
}

void ZExtInst::print(std::ostream &os) {
    os << std::format("{} = {} {} to {}", getName(), opcodeToStr(OpCode::ZEXT),
                      getOperand(0)->getType()->name, getType()->name);
}

//---

TruncInst::TruncInst(Value *value, TypeKind *type, const std::string &name) :
    Inst(OpCode::ZEXT, type, {value}, name) {}

TruncInst *TruncInst::Create(Value *value, TypeKind *type, const std::string &name) {
    return new TruncInst(value, type, name);
}

void TruncInst::print(std::ostream &os) {
    os << std::format("{} = {} {} to {}", getName(), opcodeToStr(OpCode::TRUNC),
                      getOperand(0)->getType()->name, getType()->name);
}

//---

AllocaInst::AllocaInst(TypeKind *type, const std::string &name) :
    Inst(OpCode::ALLOCA, getPtrTo(type->name), {}, name),
    valType(type) {}

AllocaInst *AllocaInst::Create(TypeKind *type, const std::string &name) {
    return new AllocaInst(type, name);
}

TypeKind *AllocaInst::getValType() {
    return valType;
}

void AllocaInst::print(std::ostream &os) {
    os << std::format("{} = {} {}", getName(), opcodeToStr(OpCode::ALLOCA), valType->name);
}

//---

LoadInst::LoadInst(TypeKind *type, Value *ptr, const std::string &name) :
    Inst(OpCode::LOAD, type, {ptr}, name) {}

LoadInst *LoadInst::Create(TypeKind *type, Value *ptr, const std::string &name) {
    return new LoadInst(type, ptr, name);
}

Value *LoadInst::getValue() {
    return getOperand(0);
}

void LoadInst::print(std::ostream &os) {
    os << std::format("{} = {} {} <- ", getName(), opcodeToStr(OpCode::LOAD), getType()->name);
    getValue()->printAsOperand(os);
}

//---

StoreInst::StoreInst(Value *value, Value *ptr) :
    Inst(OpCode::STORE, value->getType(), {value, ptr}, "") {}

StoreInst *StoreInst::Create(Value *value, Value *ptr) {
    return new StoreInst(value, ptr);
}

Value *StoreInst::getValue() {
    return getOperand(0);
}

Value *StoreInst::getDest() {
    return getOperand(1);
}

void StoreInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::STORE));
    getValue()->printAsOperand(os);
    os << " -> ";
    getDest()->printAsOperand(os);
}

//---

CallInst::CallInst(Func *callee, std::vector<Value *> args, const std::string &name) :
    Inst(OpCode::CALL, callee->getReturnType(), args, name),
    callee(callee) {}

CallInst *CallInst::Create(Func *callee, std::vector<Value *> args, const std::string &name) {
    return new CallInst(callee, args, name);
}

Func *CallInst::getCallee() {
    return callee;
}

void CallInst::print(std::ostream &os) {
    TypeKind *retType = callee->getReturnType();

    os << std::format("{} = call {} @{} (", getName(), retType->name, callee->getName());

    for (auto &argVal : getOperands()) {
        argVal->printAsOperand(os);

        if (argVal != getOperands().back()) os << ", ";
    }

    os << ')';
}

//---

BuiltInInst::BuiltInInst(std::vector<Value *> args, const BuiltIn &kind,
    const std::string &name) :
    Inst(OpCode::CALL, getVoidTy(), args, name),
    kind(kind) {}

BuiltInInst *BuiltInInst::Create(std::vector<Value *> args, const BuiltIn &kind,
    const std::string &name) {
    return new BuiltInInst(args, kind, name);
}

BuiltIn &BuiltInInst::getKind() {
    return kind;
}

void BuiltInInst::print(std::ostream &os) {
    os << std::format("call {} @{} (", getType()->name, getBuiltInName(kind));

    for (auto &argVal : getOperands()) {
        argVal->printAsOperand(os);

        if (argVal != getOperands().back()) os << ", ";
    }

    os << ')';
}

//---

CondBrInst::CondBrInst(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB) :
    Inst(OpCode::BRC, nullptr, {condn}, ""),
    trueBB(trueBB),
    falseBB(falseBB) {}

CondBrInst *CondBrInst::Create(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB) {
    return new CondBrInst(condn, trueBB, falseBB);
}

Value *CondBrInst::getCond() {
    return getOperand(0);
}

BasicBlock *CondBrInst::getThenBlock() {
    return trueBB;
}

BasicBlock *CondBrInst::getElseBlock() {
    return falseBB;
}

void CondBrInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::BRC));
    getCond()->printAsOperand(os);
    os << std::format(", {}, {}", getThenBlock()->getName(), getElseBlock()->getName());
}

//---

UnCondBrInst::UnCondBrInst(BasicBlock *thenBB) :
    Inst(OpCode::BR, nullptr, {}, ""),
    then(thenBB) {}

UnCondBrInst *UnCondBrInst::Create(BasicBlock *thenBB) {
    return new UnCondBrInst(thenBB);
}

BasicBlock *UnCondBrInst::getThenBlock() {
    return then;
}

void UnCondBrInst::print(std::ostream &os) {
    os << std::format("{} {}", opcodeToStr(OpCode::BR), then->getName());
}

//---

PHINode::PHINode(TypeKind *type, unsigned numEdges, const std::string &name, BasicBlock *bb) :
    Inst(OpCode::PHI, type, {}, name),
    numEdges(numEdges) {

    pairs.reserve(numEdges);
    setParent(bb);
}

PHINode *PHINode::Create(TypeKind *type, unsigned numEdges,
        const std::string &name, BasicBlock *bb) {
    return new PHINode(type, numEdges, name, bb);
}

std::vector<BasicBlock*> PHINode::blocks() {
    std::vector<BasicBlock*> bbs;
    for (auto &[_, bb]: pairs) bbs.push_back(bb);

    return bbs;
}

std::vector<std::pair<Value*, BasicBlock*>> &PHINode::getPairs() {
    return pairs;
}

std::pair<Value*, BasicBlock*> &PHINode::getPair(unsigned id) {
    return pairs[id];
}

void PHINode::addIncoming(Value *value, BasicBlock *bb) {
    addOperand(value);
    pairs.push_back(std::make_pair(value, bb)); 
}

void PHINode::updPair(unsigned i, Value *value) {
    pairs[i].first = value;    
}

unsigned PHINode::getNumEdges() {
    return numEdges;
}

void PHINode::remBlock(BasicBlock *bb) {
    unsigned i = 0;
    for (auto &[_, block]: pairs) {
        if (block == bb) break;
        i++;
    }

    if (i == pairs.size()) return;
    pairs.erase(pairs.begin() + i);
    remOperand(i);

    numEdges--; 
}

void PHINode::print(std::ostream &os) {
    os << std::format("{} = {} {} ", getName(), opcodeToStr(OpCode::PHI), getType()->name);

    for (auto &[value, bb]: pairs) {
        os << "[";
        value->printAsOperand(os);
        os << std::format(", {}] ", bb->getName());
    }
}

//---

ReturnInst::ReturnInst(Value *value) :
    Inst(OpCode::RET, value->getType(), {value}, "") {}

ReturnInst::ReturnInst() :
    Inst(OpCode::RET, getVoidTy(), {}, "") {}

ReturnInst *ReturnInst::Create(Value *value) {
    if (value) return new ReturnInst(value);
    else return new ReturnInst();
}

void ReturnInst::print(std::ostream &os) {
    os << std::format("{} ", opcodeToStr(OpCode::RET));

    if (getNumOperands() > 0) getOperand(0)->printAsOperand(os);
    else os << getType()->name;
}

//---

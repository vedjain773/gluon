#include "IR/Value.hpp"
#include "IR/Func.hpp"

#include <format>
#include <algorithm>

//---

Use::Use(Inst *inst, unsigned no):
    inst(inst), no(no) {}

Use *Use::Create(Inst *inst, unsigned no) {
    return new Use(inst, no);
}

Inst *Use::getInst() {
    return inst;
}

unsigned Use::getOpNo() {
    return no;
}

void Use::replaceUseWith(Value *value) {
    Inst *user = inst;
    unsigned idx = no;

    Value *currValue = inst->getOperand(no);
    currValue->removeUse(inst, no);

    user->setOperand(idx, value);
    value->addUse(user, idx);
}

//---

Value::Value(TypeKind *type, ValueKind vkind, const std::string &name) :
    name(name),
    type(type),
    vkind(vkind) {}

void Value::addUse(Inst *inst, unsigned no) {
    valUses.push_back(Use::Create(inst, no));
}

void Value::addUse(Use *use) {
    valUses.push_back(use);
}

void Value::replaceAllUsesWith(Value *value) {
    for (auto &use: valUses) {
        use->replaceUseWith(value); 
    }
}

void Value::removeUse(Inst *inst, unsigned idx) {
    for (auto it = valUses.begin(); it != valUses.end(); ++it) {
        if ((*it)->getInst() == inst && (*it)->getOpNo() == idx) {
            valUses.erase(it);
            return;
        }
    }
}

std::vector<Use*> &Value::uses() {
    return valUses;
}

std::vector<Inst*> Value::users() {
    std::vector<Inst*> users;

    for (auto &use: valUses)
        users.push_back(use->getInst());

    return users;
}

ValueKind Value::getValueKind() {
    return vkind;
}

TypeKind *Value::getType() {
    return type;
}

void Value::setName(const std::string &name) {
    this->name = name; 
}

std::string Value::getName() {
    return name;
}

//---

ConstantInt::ConstantInt(TypeKind *intType, uint64_t value, const std::string &name) :
    Value(intType, ValueKind::Constant, name),
    value(value) {}

ConstantInt *ConstantInt::Create(TypeKind *intType, uint64_t value, const std::string &name) {
    return new ConstantInt(intType, value, name);
}

uint64_t ConstantInt::getValue() {
    return value;
}

void ConstantInt::printAsOperand(std::ostream &os) {
    os << std::format("{} {}", getType()->name, value);
}

//---

Arg::Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo) :
    Value(type, ValueKind::Argument, name),
    func(F),
    argNo(argNo) {}

Arg *Arg::Create(TypeKind *type, const std::string &name, Func *F, unsigned argNo) {
    return new Arg(type, name, F, argNo);
}

unsigned Arg::getArgNo() {
    return argNo;
}

//---

TypeKind *getVoidTy() {
    return getType("void");
}

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

void Use::setOpNo(unsigned no) {
    this->no = no;
}

void Use::replaceUseWith(Value *value) {
    Inst *user = inst;
    unsigned idx = no;

    Value *currValue = inst->getOperand(no);
    currValue->removeUse(inst, no);

    user->setOperand(idx, value);
    if (PHINode *phi = dynamic_cast<PHINode*>(user)) phi->updPair(idx, value);

    value->addUse(user, idx);
}

//---

Value::Value(TypeKind *type, ValueKind vkind, const std::string &name) :
    name(name),
    type(type),
    vkind(vkind) {}

void Value::addUse(Inst *inst, unsigned no) {
    uses.push_back(Use::Create(inst, no));
}

Use *Value::getUse(Inst *inst, unsigned no) {
    for (auto &use: uses) {
        Inst *user = use->getInst();
        unsigned opno = use->getOpNo();

        if (user == inst && opno == no) return use;
    }
}

void Value::replaceAllUsesWith(Value *value) {
    for (auto &use: uses) {
        use->replaceUseWith(value); 
    }
}

void Value::removeUse(Inst *inst, unsigned idx) {
    for (auto it = uses.begin(); it != uses.end(); ++it) {
        if ((*it)->getInst() == inst && (*it)->getOpNo() == idx) {
            uses.erase(it);
            return;
        }
    }
}

std::vector<Use*> &Value::getUses() {
    return uses;
}

std::vector<Inst*> Value::getUsers() {
    std::vector<Inst*> users;

    for (auto &use: uses)
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

ArrayLit::ArrayLit(TypeKind *type, const std::string &name) :
    Value(type, ValueKind::Aggregate, name) {}

ArrayLit *ArrayLit::Create(TypeKind *type, const std::string &name) {
    return new ArrayLit(type, name);
}

void ArrayLit::addValue(Value *value) {
    values.push_back(value);
}

Value *ArrayLit::getValue(int idx) {
    return values[idx];
}

void ArrayLit::printAsOperand(std::ostream &os) {
    os << std::format("{} [", getType()->name);
    
    for (Value *value: values) {
        value->print(os);

        if (value != values.back()) os << ", "; 
    }

    os << "]";
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

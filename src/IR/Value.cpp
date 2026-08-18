#include "IR/Value.hpp"
#include "IR/Func.hpp"
#include <format>

//---

Value::Value(TypeKind *type, ValueKind vkind, unsigned id, const std::string &name)
    :name(name), type(type), vkind(vkind), id(id) {}

ValueKind Value::getValueKind() { return vkind; }

TypeKind *Value::getType() { return type; }

std::string Value::getName() { return name; }

//---

ConstantInt::ConstantInt(TypeKind *intType, unsigned id, uint64_t value, const std::string &name)
    : Value(intType, ValueKind::Constant, id, name), value(value) {}

ConstantInt *ConstantInt::Create(TypeKind *intType, unsigned id, uint64_t value,
        const std::string &name)
{
    return new ConstantInt(intType, id, value, name);    
}

void ConstantInt::printAsOperand(std::ostream &os) {
    os << std::format("{} {}", getType()->name, value);
}

//---

Arg::Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo)
    :Value(type, ValueKind::Argument, 0, name), func(F), argNo(argNo) {}

Arg *Arg::Create(TypeKind *type, const std::string &name, Func *F, unsigned argNo) {
    return new Arg(type, name, F, argNo); 
}

//---

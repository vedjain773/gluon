#include "IR/Value.hpp"
#include "IR/Func.hpp"

//---

Value::Value(TypeKind *type, ValueKind vkind, unsigned id, const std::string &name)
    :name(name), type(type), vkind(vkind), id(id) {}

ValueKind Value::getValueKind() { return vkind; }

TypeKind *Value::getType() { return type; }

//---

ConstantInt::ConstantInt(TypeKind *intType, unsigned id, uint64_t value, const std::string &name)
    : Value(intType, ValueKind::Constant, id, name), value(value) {} 

//---

Arg::Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo)
    :Value(type, ValueKind::Argument, 0, name), func(F), argNo(argNo) {} 

//---

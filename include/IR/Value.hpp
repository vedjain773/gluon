#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <cstdint>
#include "utils/Scope.hpp"

class Func;

enum class ValueKind {
    Constant,
    Instruction,
    Argument
};

class Value {
  private:
    std::string name;
    TypeKind *type;
    ValueKind vkind;
    unsigned id;

  protected:
    Value(TypeKind *type, ValueKind vkind, unsigned id, const std::string &name);

  public:
    virtual ~Value() = default;
    ValueKind getValueKind();
    TypeKind *getType();
};

class ConstantInt : public Value {
  private:
    uint64_t value;
  
  public:
    ConstantInt(TypeKind *intType, unsigned id, uint64_t value, const std::string &name);
};

class Arg: public Value {
  private:
    Func *func;
    unsigned argNo;

  public:
    Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo);
};

#endif

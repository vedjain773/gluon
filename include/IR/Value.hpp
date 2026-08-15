#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <cstdint>
#include "utils/Scope.hpp"

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
    void setValueKind(ValueKind valueKind);

  public:
    ValueKind getValueKind();
    TypeKind *getType();
};

class ConstantInt : public Value {
  private:
    uint64_t value;
    uint8_t bitWidth;

  public:
    ConstantInt(uint64_t value, uint8_t bitWidth);
};

#endif

#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <cstdint>
#include <format>
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

  protected:
    Value(TypeKind *type, ValueKind vkind, const std::string &name);

  public:
    virtual ~Value() = default;
    ValueKind getValueKind();
    TypeKind *getType();
    std::string getName();

    virtual void printAsOperand(std::ostream &os) {
        os << std::format("{} {}", type->name, name.empty() ? "<unnamed>" : name);
    } 

    virtual void print(std::ostream &os) {
        printAsOperand(os);
    }
};

class ConstantInt : public Value {
  private:
    uint64_t value;
  
  public:
    ConstantInt(TypeKind *intType, uint64_t value, const std::string &name);
    static ConstantInt *Create(TypeKind *intType, uint64_t value, const std::string &name = "");

    void printAsOperand(std::ostream &os);
};

class Arg: public Value {
  private:
    Func *func;
    unsigned argNo;

  public:
    Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo);
    static Arg *Create(TypeKind *type, const std::string &name, Func *F, unsigned argNo);
};

TypeKind *getVoidTy();

#endif

#ifndef VALUE_H
#define VALUE_H

#include <string>
#include <cstdint>
#include <vector>
#include <format>
#include "utils/Scope.hpp"

class Func;
class Inst;
class Value;

enum class ValueKind {
    Constant,
    Instruction,
    Argument
};

class Use {
  private:
    Inst *inst;
    unsigned no;
    Use(Inst *inst, unsigned no);

  public:
    static Use *Create(Inst *inst, unsigned no);
    Inst *getInst();
    unsigned getOpNo();

    void replaceUseWith(Value *value);
};

class Value {
  private:
    std::string name;
    TypeKind *type;
    ValueKind vkind;

    std::vector<Use *> uses;

  protected:
    Value(TypeKind *type, ValueKind vkind, const std::string &name);

  public:
    virtual ~Value() = default;

    void addUse(Inst *inst, unsigned no);
    void addUse(Use *use); 

    void removeUse(Inst *inst, unsigned idx);
    void replaceAllUsesWith(Value *value);

    std::vector<Use*> &getUses();
    std::vector<Inst*> getUsers();
    
    ValueKind getValueKind();
    TypeKind *getType();

    void setName(const std::string &name);
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

    uint64_t getValue();
    void printAsOperand(std::ostream &os);
};

class Arg: public Value {
  private:
    Func *func;
    unsigned argNo;

  public:
    Arg(TypeKind *type, const std::string &name, Func *F, unsigned argNo);
    static Arg *Create(TypeKind *type, const std::string &name, Func *F, unsigned argNo);

    unsigned getArgNo();
};

TypeKind *getVoidTy();

#endif

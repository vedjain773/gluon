#ifndef INST_H
#define INST_H

#include "IR/Value.hpp"

class BasicBlock;
class Func;

enum class OpCode {
    //Unary
    NOT, NEG,

    //Binary
    ADD, SUB, MUL, DIV, REM,
    GT, GTE, LT, LTE,
    EQ, NEQ,
    LAND, LOR,

    //Declaration
    ALLOCA,

    //Assignment
    LOAD, STORE,

    //Terminators
    RET, BR, BRC,

    //Call
    CALL
};

class Inst : public Value {
  private:
    OpCode opcode; 
    BasicBlock *parent;
    std::vector<Value*> operands;

  protected:
    Inst(OpCode op, TypeKind *type, std::vector<Value*> ops, const std::string &name);

  public:
    OpCode getOpCode();
    BasicBlock *getParent();
    void setParent(BasicBlock *bb);

    unsigned getNumOperands();
    Value *getOperand(unsigned i);
    void setOperand(unsigned i, Value *v);

    bool isTerminator();

    void print(std::ostream &os);
};

class UnaryInst: public Inst {
  private:
    UnaryInst(OpCode opcode, Value* operand, const std::string &name);
  
  public:
    static UnaryInst *Create(OpCode opcode, Value* operand, const std::string &name);
    Value *getUnaryOper();
};

class BinaryInst: public Inst {
  private:
    BinaryInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name);

  public:
    static BinaryInst *Create(OpCode opcode, Value* lhs, Value *rhs, const std::string &name);
    Value *getLHS();
    Value *getRHS();
};

class CompInst: public Inst {
  private:
    CompInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name);

  public:
    static CompInst *Create(OpCode opcode, Value* lhs, Value *rhs, const std::string &name);
    Value *getLHS();
    Value *getRHS();
};

class AllocaInst: public Inst {
  private:
    AllocaInst(TypeKind *type, const std::string &name);
  
  public:
    static AllocaInst *Create(TypeKind *type, const std::string &name);
};

class LoadInst: public Inst {
  private:
    LoadInst(TypeKind *type, Value *value, const std::string &name);

  public:
    static LoadInst *Create(TypeKind *type, Value *value, const std::string &name);
    TypeKind *getType();
    Value *getValue();
};

class StoreInst: public Inst {
  private:
    StoreInst(Value *val, Value *dest);

  public:
    static StoreInst *Create(Value *val, Value *dest);
    Value *getSrc();
    Value *getDest();
};

class CallInst: public Inst {
  private:
    Func *callee;
    std::vector<Arg*> args;
    CallInst(Func *callee, std::vector<Value*> args, const std::string &name);

  public:
    static CallInst *Create(Func *callee, std::vector<Value*> args, const std::string &name);
    Func *getCallee();
};

class CondBrInst: public Inst {
  private:
    BasicBlock *trueBB;
    BasicBlock *falseBB;
    CondBrInst(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB);

  public:
    static CondBrInst *Create(Value *condn, BasicBlock *trueBB, BasicBlock *falseBB);
    Value *getCond();
    BasicBlock *getThenBlock();
    BasicBlock *getElseBlock();
};

class UnCondBrInst: public Inst {
  private:
    BasicBlock *then;
    UnCondBrInst(BasicBlock *then);

  public:
    static UnCondBrInst *Create(BasicBlock *then);
    BasicBlock *getThenBlock();
};

class ReturnInst: public Inst {
  private:
    ReturnInst(Value *value);

  public:
    static ReturnInst *Create(Value *value);
};

#endif

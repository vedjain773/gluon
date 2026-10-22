#ifndef INST_H
#define INST_H

#include "IR/Value.hpp"

class BasicBlock;
class Func;

enum class OpCode: unsigned {
    //Unary
    NOT, NEG,

    //Binary
    ADD, SUB, MUL, DIV, REM,
    GT, GTE, LT, LTE,
    EQ, NEQ,
    LAND, LOR,

    //Ptr-arith
    GEP,

    //Cast
    ZEXT, TRUNC,

    //Declaration
    ALLOCA,

    //Assignment
    LOAD, STORE,

    //Terminators
    RET, BR, BRC,

    //PHI nodes
    PHI,

    //Call
    CALL
};

std::string opcodeToStr(OpCode opcode);

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
    void eraseFromParent();

    unsigned getNumOperands();
    Value *getOperand(unsigned i);
    std::vector<Value*> &getOperands();
    
    void setOperand(unsigned i, Value *v);
    void addOperand(Value *value);
    void remOperand(unsigned i);

    bool isBinary();
    bool isTerminator();
};

class UnaryInst: public Inst {
  private:
    UnaryInst(OpCode opcode, Value* value, const std::string &name);
  
  public:
    static UnaryInst *Create(OpCode opcode, Value* value, const std::string &name);
    Value *getUnaryOper();

    void print(std::ostream &os);
};

class BinaryInst: public Inst {
  private:
    BinaryInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name);

  public:
    static BinaryInst *Create(OpCode opcode, Value* lhs, Value *rhs, const std::string &name);
    Value *getLHS();
    Value *getRHS();

    void print(std::ostream &os);
};

class CompInst: public Inst {
  private:
    CompInst(OpCode opcode, Value *lhs, Value *rhs, const std::string &name);

  public:
    static CompInst *Create(OpCode opcode, Value* lhs, Value *rhs, const std::string &name);
    Value *getLHS();
    Value *getRHS();

    void print(std::ostream &os);
};

class GEPInst: public Inst {
  private:
    TypeKind *srcType;
    TypeKind *getResultType(unsigned size, TypeKind *ptr);
    std::vector<Value *> getOps(Value *ptr, std::vector<Value *> idxList);

    GEPInst(TypeKind *type, Value *ptr, std::vector<Value*> idxList, const std::string &name);

  public:
    static GEPInst *Create(TypeKind *type, Value *ptr, std::vector<Value*> idxList,
            const std::string &name);

    TypeKind *getSrcType();
    Value *getPointerOp();

    unsigned getNumIndices();
    std::vector<Value*> getIndices();

    void print(std::ostream &os);
};

class ZExtInst: public Inst {
  private:
    ZExtInst(Value *value, TypeKind *type, const std::string &name);

  public:
    static ZExtInst *Create(Value *value, TypeKind *type, const std::string &name);
    
    void print(std::ostream &os);
};

class TruncInst: public Inst {
  private:
    TruncInst(Value *value, TypeKind *type, const std::string &name);

  public:
    static TruncInst *Create(Value *value, TypeKind *type, const std::string &name);
    
    void print(std::ostream &os);
};

class AllocaInst: public Inst {
  private:
    TypeKind *valType;
    AllocaInst(TypeKind *type, const std::string &name);
  
  public:
    static AllocaInst *Create(TypeKind *type, const std::string &name);
    TypeKind *getValType();

    void print(std::ostream &os);
};

class LoadInst: public Inst {
  private:
    LoadInst(TypeKind *type, Value *value, const std::string &name);

  public:
    static LoadInst *Create(TypeKind *type, Value *value, const std::string &name);
    Value *getValue();

    void print(std::ostream &os);
};

class StoreInst: public Inst {
  private:
    StoreInst(Value *value, Value *dest);

  public:
    static StoreInst *Create(Value *value, Value *dest);
    Value *getValue();
    Value *getDest();

    void print(std::ostream &os);
};

class CallInst: public Inst {
  private:
    Func *callee;
    CallInst(Func *callee, std::vector<Value*> args, const std::string &name);

  public:
    static CallInst *Create(Func *callee, std::vector<Value*> args, const std::string &name);
    Func *getCallee();

    void print(std::ostream &os);
};

class BuiltInInst: public Inst {
  private:
    std::string funcName;
    BuiltInInst(std::vector<Value*> args, const std::string &funcName, const std::string &name);

  public:
    static BuiltInInst *Create(std::vector<Value*> args,
                               const std::string &funcName, const std::string &name);
    std::string &getName();

    void print(std::ostream &os);
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

    void print(std::ostream &os);
};

class UnCondBrInst: public Inst {
  private:
    BasicBlock *then;
    UnCondBrInst(BasicBlock *then);

  public:
    static UnCondBrInst *Create(BasicBlock *then);
    BasicBlock *getThenBlock();

    void print(std::ostream &os);
};

class PHINode: public Inst {
  private:
    std::vector<std::pair<Value*, BasicBlock*>> pairs;
    unsigned numEdges;

    PHINode(TypeKind *type, unsigned numEdges, const std::string &name, BasicBlock *bb);

  public:
    static PHINode *Create(TypeKind *type, unsigned numEdges,
            const std::string &name, BasicBlock *bb);
  
    std::vector<BasicBlock*> blocks();
    std::vector<std::pair<Value*, BasicBlock*>> &getPairs();

    void addIncoming(Value *value, BasicBlock *bb);
    void updPair(unsigned i, Value *value);
    void remBlock(BasicBlock *bb);

    std::pair<Value*, BasicBlock*> &getPair(unsigned id);
    unsigned getNumEdges();

    void print(std::ostream &os);
};

class ReturnInst: public Inst {
  private:
    ReturnInst(Value *value);
    ReturnInst();

  public:
    static ReturnInst *Create(Value *value);

    void print(std::ostream &os);
};

#endif

#ifndef INST_H
#define INST_H

#include "IR/Value.hpp"

enum class OpCode {
    //Unary
    NOT, NEG,

    //Binary
    ADD, SUB, MUL, DIV, REM,
    GT, GTE, LT, LTE,
    EQ, NEQ,
    LAND, LOR,

    //Assignment
    LOAD, STORE,

    //Terminators
    RET, BR, BRC
};

class Instruction : public Value {
private:
    OpCode opcode; 
    BasicBlock *parent;
    std::vector<Value*> operands;

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

#endif

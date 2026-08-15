#ifndef IRBUILDER_H
#define IRBUILDER_H

#include "Module.hpp"
#include "Func.hpp"
#include "BasicBlock.hpp"
#include "Inst.hpp"
#include "Value.hpp"

class IRBuilder {
private:
    Module *currModule;
    Func *currFunc;
    BasicBlock *currBasicBlock;

public:
    IRBuilder(Module *module);

    void setInsertPoint(BasicBlock *bb);
    BasicBlock *getInsertBlock();

    Value *createBinOp(BinOpKind op, Value *lhs, Value *rhs, const std::string &name = "");
    Value *createCmp(CmpKind pred, Value *lhs, Value *rhs, const std::string &name = "");
    Value *createNeg(Value *value, const std::string &name = "");
    Value *createNot(Value *value, const std::string &name = "");

    Value *createAlloca(TypeKind *type, const std::string &name = "");
    Value *createLoad(TypeKind *type, Value *ptr, const std::string &name = "");
    Instruction *createStore(Value *value, Value *dest);

    Value *createCast(CastKind kind, Value *value, TypeKind *destType, 
            const std::string &name = "");
    Value *createCall(Func *callee, std::vector<Value*> args, const std::string &name = "");

    Instruction *createRet();
    Instruction *createRet(Value *value);
    Instruction *createBr(BasicBlock *dest);
    Instruction *createCondBr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB);

    void insertBasicBlock(std::unique_ptr<BasicBlock> bb);
    void insertFunc(std::unique_ptr<Func> func);
};

#endif

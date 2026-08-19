#ifndef IRBUILDER_H
#define IRBUILDER_H

#include "IR/Module.hpp"
#include "IR/Func.hpp"
#include "IR/BasicBlock.hpp"
#include "IR/Inst.hpp"
#include "IR/Value.hpp"

class IRBuilder {
  private:
    Module *currModule;
    Func *currFunc;
    BasicBlock *currBasicBlock;

  public:
    IRBuilder(Module *module);

    void setInsertPoint(BasicBlock *bb);
    BasicBlock *getInsertBlock();

    Value *createBinOp(OpCode op, Value *lhs, Value *rhs, const std::string &name = "");
    Value *createCmp(OpCode pred, Value *lhs, Value *rhs, const std::string &name = "");
    Value *createNeg(Value *value, const std::string &name = "");
    Value *createNot(Value *value, const std::string &name = "");

    Value *createZExt(Value *value, TypeKind *target, const std::string &name = "");

    AllocaInst *createAlloca(TypeKind *type, const std::string &name = "");
    AllocaInst *createEntryAlloca(TypeKind *type, const std::string &name = "");
    
    LoadInst *createLoad(TypeKind *type, Value *ptr, const std::string &name = "");
    StoreInst *createStore(Value *value, Value *dest);

    CallInst *createCall(Func *callee, std::vector<Value*> args, const std::string &name = "");

    ReturnInst *createRet(Value *value);
    UnCondBrInst *createBr(BasicBlock *then);
    CondBrInst *createCondBr(Value *cond, BasicBlock *trueBB, BasicBlock *falseBB);
};

#endif

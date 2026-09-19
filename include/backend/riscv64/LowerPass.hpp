#ifndef LOWER_H
#define LOWER_H

#include "backend/riscv64/mModule.hpp"
#include "IR/Module.hpp"

#include <unordered_map>

namespace RISCV {

class LowerPass {
  private:
    Module *module;

    mModule *mmod;
    mFunc *currFunc;
    mBlock *currBlock;

    std::unordered_map<Value*, VirtReg*> virtualRegTable;
    std::unordered_map<Value*, StackSlot*> stackSlotTable;
    unsigned currentRegNo;

    mOperand *insertReg(Value *value);
    mOperand *handleValue(Value *value); 
    mOperand *materialize(mOperand *oper);

    void handleRet(Inst *inst);
    void handleBinOp(Inst *inst, const OpCode &code);
    void handleCmpOp(Inst *inst, const OpCode &code);

    void handleAlloca(Inst *inst);
    void handleLoad(Inst *inst);
    void handleStore(Inst *inst);

  public:
    LowerPass (Module *module);
    mModule *getModule();

    void lower();
        
    void lowerFunc(Func *func);
    void lowerBlock(BasicBlock *block);
    void lowerInst(Inst *inst);

    void print(std::ostream &os);
};

Code getCode(const OpCode &opc);

}

#endif

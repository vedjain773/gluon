#ifndef LOWER_H
#define LOWER_H

#include "backend/riscv64/mModule.hpp"
#include "IR/Module.hpp"

namespace RISCV {

class LowerPass {
  private:
    Module *module;

    mModule *mmod;
    mFunc *currFunc;
    mBlock *currBlock;

    void handleRet(Value *value);

  public:
    LowerPass (Module *module);
    mModule *getModule();

    void lower();
        
    void lowerFunc(Func *func);
    void lowerBlock(BasicBlock *block);
    void lowerInst(Inst *inst);

    void print(std::ostream &os);
};

}

#endif

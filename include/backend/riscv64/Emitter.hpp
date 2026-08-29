#ifndef EMITTER_H
#define EMITTER_H

#include "backend/riscv64/mModule.hpp"

namespace RISCV {

class Emitter {
  private:
    mModule *module;
    std::ostream &os;

    void emitFunc(mFunc *func);
    void emitBlock(mBlock *block);

    void emitInst(mInst *inst);

    void emitLI(mOperand *reg, mOperand *imm);

    void emitOper(mOperand *oper);

  public:
    Emitter(mModule *module, std::ostream &os);
    void emit(); 
};

}

#endif

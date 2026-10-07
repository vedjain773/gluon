#ifndef EMITTER_H
#define EMITTER_H

#include "backend/mModule.hpp"
#include <vector>

namespace RISCV {

class Emitter {
  private:
    mModule *module;
    std::ostream &os;

    std::vector<mBlock*> blockIDs;
    unsigned getId(mBlock* block);
    std::string getInstStr(const Code &code);

    void emitFunc(mFunc *func);
    void emitBlock(mBlock *block);

    void emitInst(mInst *inst);
    void emitBr(mInst *inst, Code code);
    void emitCall(mInst *inst);

    void emitOper(mOperand *oper);

  public:
    Emitter(mModule *module, std::ostream &os);
    void emit(); 
};

}

#endif

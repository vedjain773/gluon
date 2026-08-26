#ifndef MBUILDER_H
#define MBUILDER_H

#include "backend/riscv64/mInst.hpp"
#include "backend/riscv64/mFunc.hpp"
#include "backend/riscv64/mBlock.hpp"

namespace RISCV {

class mBuilder {
  private:
    mModule *currModule;
    mFunc *currFunc;
    mBlock *currBlock;

  public:
    mBuilder(mModule *currModule);

    void setInsertPoint(mBlock *block);
    mBlock *getInsertPoint();

    mInst *createInst(OpCode opcode, std::vector<mOperand> operands);
};

}

#endif

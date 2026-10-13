#ifndef PHIPASS_H
#define PHIPASS_H

#include "backend/mModule.hpp"
#include "backend/mFunc.hpp"

namespace RISCV {

class PhiPass {
  private:
    mModule *mmod;

    void addDef(std::pair<mOperand*, mBlock*> &pair, mOperand *oper);
    void insertDefs(mFunc *func);
    void remPHI(mFunc *func);

  public:
    PhiPass(mModule *mmod);
    void run();
};

}

#endif

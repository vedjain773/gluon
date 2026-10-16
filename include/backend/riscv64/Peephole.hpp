#ifndef PEEPHOLE_H
#define PEEPHOLE_H

#include "backend/mModule.hpp"

namespace RISCV {

class Peephole {
  private:
    mModule *mod;
    void applyIdentity(mInst *inst);

  public:
    Peephole(mModule *mod);
    void redInst();
};

};

#endif

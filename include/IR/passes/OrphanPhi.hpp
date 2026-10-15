#ifndef ORPHANPHI_H
#define ORPHANPHI_H

#include "IR/passes/Pass.hpp"

class OrphanPhi: public Pass {
  public:
    void run(Func *func);
};

#endif

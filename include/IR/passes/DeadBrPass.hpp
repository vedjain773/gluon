#ifndef DEADBR_H
#define DEADBR_H

#include "IR/passes/Pass.hpp"
#include "IR/Value.hpp"

class DeadBrPass: public Pass {
  public:
    void run(Func *F);
};

#endif

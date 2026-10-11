#ifndef PASS_H
#define PASS_H

#include "IR/Func.hpp"

class Pass {
  public:
    virtual void run(Func *func) = 0;
};

#endif

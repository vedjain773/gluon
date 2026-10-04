#ifndef FRAMEPASS_H
#define FRAMEPASS_H

#include "backend/riscv64/mModule.hpp"
#include "backend/riscv64/mFunc.hpp"

namespace RISCV {

class FramePass {
  private:
    mModule *mmod;
    void calcOffsets(mFunc *func); 

  public:
    FramePass(mModule *mmod);
    void run();
};

}

#endif

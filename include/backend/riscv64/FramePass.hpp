#ifndef FRAMEPASS_H
#define FRAMEPASS_H

#include "backend/mModule.hpp"
#include "backend/mFunc.hpp"

namespace RISCV {

class FramePass {
  private:
    mModule *mmod;
    
    void calcOffsets(mFunc *func); 
    void expand(mFunc *func);

  public:
    FramePass(mModule *mmod);
    void run();
};

}

#endif

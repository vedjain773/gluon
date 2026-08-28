#ifndef MMODULE_H
#define MMODULE_H

#include "backend/riscv64/mFunc.hpp"
#include "IR/Module.hpp"

namespace RISCV {

class mModule {
  private:
    std::string name;
    std::vector<std::unique_ptr<mFunc>> functions;

  public:
    mModule(Module &mod);
    static mModule *Create(Module &mod);
    
    mFunc *appendFunc(std::unique_ptr<mFunc> func);

    void print(std::ostream &os);
};
    
}

#endif

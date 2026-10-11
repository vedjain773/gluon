#ifndef OPT_H
#define OPT_H

#include "IR/passes/Pass.hpp"
#include "IR/Module.hpp"
#include <vector>

class Opt {
  private:
    Module *mod;
    std::vector<Pass*> passes;

  public:
    Opt(Module *mod);

    void addPass(Pass *pass);
    void optimize();
};

#endif

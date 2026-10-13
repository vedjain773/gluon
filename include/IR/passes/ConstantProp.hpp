#ifndef CONSTANTPROP_H
#define CONSTANTPROP_H

#include "IR/passes/Pass.hpp"
#include "IR/Value.hpp"

class ConstantProp: public Pass {
  private:
    ConstantInt *fold(OpCode opcode, ConstantInt *c1, ConstantInt *c2);

  public:
    void run(Func *F);
};

#endif

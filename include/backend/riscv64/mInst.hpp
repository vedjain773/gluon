#ifndef MINST_H
#define MINST_H

#include <vector>

namespace RISCV {

class mBlock;

enum class OpCode {
    // ...
};

class mOperand {
    // ...
};

class mInst {
  private:
    OpCode opcode;
    mBlock* parent;
    std::vector<mOperand> operands;

  public:
    mInst(OpCode opcode, mBlock* parent,
          std::vector<mOperand> operands);

    OpCode getOpCode();
    mBlock* getParent();

    unsigned getNumOperands();
    mOperand& getOperand(unsigned i);

    void setOperand(unsigned i, mOperand operand);
};

}
#endif

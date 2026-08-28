#ifndef MINST_H
#define MINST_H

#include "backend/riscv64/mOperand.hpp"
#include <vector>
#include <string>

namespace RISCV {

class mBlock;

class mInst {
  private:
    Code opcode;
    mBlock* parent;
    std::vector<mOperand*> operands;

  public:
    mInst(Code opcode, mBlock* parent,
          std::vector<mOperand*> operands);

    Code getOpCode();
    mBlock* getParent();

    unsigned getNumOperands();
    mOperand *getOperand(unsigned i);

    void setOperand(unsigned i, mOperand *operand);

    void print(std::ostream &os);
};

std::string codeToStr(const Code &code);

}

#endif

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
          std::vector<mOperand*> operands = {});
    virtual ~mInst() = default;

    Code getOpCode();
    mBlock* getParent();

    unsigned getNumOperands();
    mOperand *getOperand(unsigned i);

    void setOperand(unsigned i, mOperand *operand);

    virtual void print(std::ostream &os);
};

class mBrInst: public mInst {
  private:
    mBlock* label;

  public:
    mBrInst(Code opcode, mBlock *parent, mOperand *cond, mBlock *label);
    void print(std::ostream &os);
};

std::string codeToStr(const Code &code);

}

#endif

#ifndef MBLOCK_H
#define MBLOCK_H

#include "backend/riscv64/mInst.hpp"
#include "IR/BasicBlock.hpp"
#include <vector>
#include <memory>

namespace RISCV {

class mFunc;
class mInst;
class mOperand;

class mBlock {
  private:
    std::string name;
    mFunc* parent;

    std::vector<std::unique_ptr<mInst>> instructions;
    std::vector<mBlock*> preds;
    std::vector<mBlock*> succs;

  public:
    mBlock(BasicBlock &bb, mFunc* parent);

    std::string getName();
    mFunc* getParent();

    void appendInst(std::unique_ptr<mInst> inst);

    std::vector<mBlock*>& getPreds();
    std::vector<mBlock*>& getSuccs();

    void addPred(mBlock* bb);

    void print(std::ostream& os);
};

}
#endif

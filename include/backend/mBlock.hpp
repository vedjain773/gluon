#ifndef MBLOCK_H
#define MBLOCK_H

#include "backend/mInst.hpp"
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
    mBlock(const std::string &blockName, mFunc *parent);

    std::string getName();
    mFunc* getParent();

    void appendInst(std::unique_ptr<mInst> inst);
    void appendAtTop(std::unique_ptr<mInst> inst);
    std::vector<std::unique_ptr<mInst>> &getInsts();

    std::vector<mBlock*>& getPreds();
    std::vector<mBlock*>& getSuccs();

    void addPred(mBlock *bb);
    void addSucc(mBlock *bb);

    void remRet();
    void print(std::ostream& os);
};

}
#endif

#ifndef MFUNC_H
#define MFUNC_H

#include "backend/riscv64/mBlock.hpp"
#include "IR/Func.hpp"
#include <vector>
#include <memory>

namespace RISCV {

class mBlock;
class mModule;

class mFunc {
  private:
    std::string name;
    mModule *parent;
    std::vector<std::unique_ptr<mBlock>> blocks;

  public:
    mFunc(Func &func, mModule *parent);

    std::string getName();
    mModule *getParent();

    mBlock* appendBlock(std::unique_ptr<mBlock> bb);
    mBlock* getEntryBlock();

    void print(std::ostream& os);
};

}

#endif

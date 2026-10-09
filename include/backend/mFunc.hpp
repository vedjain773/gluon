#ifndef MFUNC_H
#define MFUNC_H

#include "backend/mBlock.hpp"
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
    std::vector<StackSlot*> stackSlots;
    std::vector<Reg> calleeSavedRegs;

    int frameSize = 16;

  public:
    mFunc(Func &func, mModule *parent);

    std::string getName();
    mModule *getParent();

    unsigned getNextSlotId();
    std::vector<StackSlot*> &getStackSlots();
    void insertSlot(StackSlot *slot);

    void setFrameSize(unsigned size);
    void addCalleeSavedReg(Reg reg);
    unsigned getCSRsize();

    void addPrologue();
    void addEpilogue();

    mBlock* appendBlock(std::unique_ptr<mBlock> bb);
    mBlock* getEntryBlock();

    std::vector<std::unique_ptr<mBlock>> &getBlocks();

    void print(std::ostream& os);
};

}

#endif

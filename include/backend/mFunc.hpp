#ifndef MFUNC_H
#define MFUNC_H

#include "backend/mBlock.hpp"
#include "IR/Func.hpp"
#include <vector>
#include <memory>

namespace RISCV {

class mBlock;
class mModule;
class VirtReg;

class mFunc {
  private:
    std::string name;
    mModule *parent;

    std::vector<std::unique_ptr<mBlock>> blocks;

    std::vector<StackSlot*> stackSlots;
    std::vector<Reg> calleeSavedRegs;

    std::unordered_map<VirtReg *, unsigned> spills;

    int stackArgs = 0;
    int frameSize = 16;

  public:
    mFunc(Func &func, mModule *parent);
    mFunc(const std::string &funcName, mModule *parent);

    std::string getName();
    mModule *getParent();

    unsigned getNextSlotId();
    std::vector<StackSlot*> &getStackSlots();
    StackSlot *getStackSlot(unsigned id);
    void insertSlot(StackSlot *slot);

    unsigned getSpillId(VirtReg *vreg);
    void addSpill(VirtReg *vreg);

    void setFrameSize(unsigned size);
    void addCalleeSavedReg(Reg reg);
    unsigned getCSRsize();

    void addStackArg();
    unsigned getStackArgs();

    void addPrologue();
    void addEpilogue();

    mBlock* appendBlock(std::unique_ptr<mBlock> bb);
    mBlock* getEntryBlock();

    std::vector<std::unique_ptr<mBlock>> &getBlocks();

    void print(std::ostream& os);
};

}

#endif

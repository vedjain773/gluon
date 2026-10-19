#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include <map>
#include <vector>
#include <set>
#include <bitset>

#include "backend/mModule.hpp"
#include "backend/mFunc.hpp"

#include <unordered_map>
#include <array>

#define NUM_REGS 26
#define CALLER_S 15

namespace RISCV {

struct defUses {
    std::vector<VirtReg *> vUses;
    std::vector<VirtReg *> vDefs;
    std::vector<Reg> pUses;
    std::vector<Reg> pDefs;
};

class RegAlloc {
  private:
    mModule *module;
    mFunc *func;

    std::map<mBlock*, std::set<VirtReg*>> useMap;
    std::map<mBlock*, std::set<VirtReg*>> defMap;
    std::map<mBlock*, std::set<VirtReg*>> liveInMap;
    std::map<mBlock*, std::set<VirtReg*>> liveOutMap;

    std::map<mBlock*, std::bitset<NUM_REGS>> puseMap;
    std::map<mBlock*, std::bitset<NUM_REGS>> pdefMap;
    std::map<mBlock*, std::bitset<NUM_REGS>> pliveInMap;
    std::map<mBlock*, std::bitset<NUM_REGS>> pliveOutMap;

    std::vector<mBlock*> blockVec;
    defUses getDefUses(mInst *inst);

    std::set<VirtReg*> getDiff(const std::set<VirtReg*> &s1, const std::set<VirtReg*> &s2);
    std::set<VirtReg*> getUnion(const std::set<VirtReg*> &s1, const std::set<VirtReg*> &s2);

    std::unordered_map<VirtReg*, std::set<VirtReg*>> interference;
    std::unordered_map<VirtReg*, std::set<VirtReg*>> workingITG;
    std::unordered_map<VirtReg*, std::bitset<NUM_REGS>> conflicts;
    
    void addNode(VirtReg *vreg);

    unsigned getDegree(VirtReg *vreg);
    unsigned getConflicts(VirtReg *vreg);

    std::map<Reg, unsigned> reg2Int;
    std::array<Reg, NUM_REGS> availReg;
    std::map<VirtReg*, Reg> regMap;

    bool isCalleeSaved(Reg reg);

    void addEdge(VirtReg *v1, VirtReg *v2);
    void remEdge(VirtReg *v1, VirtReg *v2);

    void virLiveAnalysis();
    void phyLiveAnalysis();
    void createITFGraph();
    void colour();
    
    void addTempRegs(std::bitset<NUM_REGS> &adjRegs, VirtReg *vreg);
    Reg getPhyReg(std::bitset<NUM_REGS> &regs);

    void reset();
    void replace(mFunc *func);
    void run(mFunc *func);

  public:
    RegAlloc(mModule *module);
    void allocate();

    void printLiveSets(std::ostream &os);
    void printITFGraph(std::ostream &os);
};

}
    
#endif

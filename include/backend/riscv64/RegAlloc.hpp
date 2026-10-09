#ifndef REG_ALLOC_H
#define REG_ALLOC_H

#include <map>
#include <vector>
#include <set>

#include "backend/mModule.hpp"
#include "backend/mFunc.hpp"

#include <unordered_map>

namespace RISCV {

class RegAlloc {
  private:
    mModule *module;
    mFunc *func;

    std::map<mBlock*, std::set<VirtReg*>> useMap;
    std::map<mBlock*, std::set<VirtReg*>> defMap;
    std::map<mBlock*, std::set<VirtReg*>> liveInMap;
    std::map<mBlock*, std::set<VirtReg*>> liveOutMap;

    std::vector<VirtReg*> liveAcrossCalls;

    std::vector<mBlock*> blockVec;

    std::set<VirtReg*> getDiff(const std::set<VirtReg*> &s1, const std::set<VirtReg*> &s2);
    std::set<VirtReg*> getInter(const std::set<VirtReg*> &s1, const std::set<VirtReg*> &s2);
    std::set<VirtReg*> getUnion(const std::set<VirtReg*> &s1, const std::set<VirtReg*> &s2);

    std::set<Reg> getRegDiff(const std::set<Reg> &s1, const std::set<Reg> &s2);

    std::unordered_map<VirtReg*, std::set<VirtReg*>> interference;
    std::unordered_map<VirtReg*, std::set<VirtReg*>> workingITG;

    std::vector<Reg> availReg;
    std::map<VirtReg*, Reg> regMap;

    bool isCalleeSaved(Reg reg);

    void addEdge(VirtReg *v1, VirtReg *v2);
    void remEdge(VirtReg *v1, VirtReg *v2);

    void performLiveAnalysis();
    void createITFGraph();
    void colour();
    
    void addTempRegs(std::set<Reg> &adjRegs, VirtReg *vreg);

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

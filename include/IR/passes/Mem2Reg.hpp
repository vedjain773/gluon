#ifndef MEM2REG_H
#define MEM2REG_H

#include "IR/passes/Pass.hpp"

#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string_view>
#include <vector>

class Mem2Reg : public Pass {
  private:
    std::set<BasicBlock*> blockList;
    std::map<BasicBlock*, std::set<BasicBlock*>> domSets;
    std::map<BasicBlock *, BasicBlock *> iDoms;
    std::map<BasicBlock*, std::set<BasicBlock*>> domTree;
    std::map<BasicBlock*, std::set<BasicBlock*>> domFrontier;
    std::map<BasicBlock*, std::set<BasicBlock*>> iDF;

    std::vector<BasicBlock*> blockVecList;
    std::map<Value *, std::set<PHINode *>> valPhiPos;
    std::map<Value *, std::stack<Value *>> allocaValStack;
    std::map<Value *, int> counter;

    // Live analysis
    std::map<BasicBlock *, std::set<Value*>> UseMap;
    std::map<BasicBlock *, std::set<Value*>> DefMap;
    std::map<BasicBlock *, std::set<Value*>> LiveInMap;
    std::map<BasicBlock *, std::set<Value*>> LiveOutMap;

    std::set<Value *> promotableAllocas; 

    // Set operations
    std::set<Value*> getDiff(const std::set<Value*> &vs1, const std::set<Value*> &vs2);
    std::set<Value*> getUnion(const std::set<Value*> &vs1, const std::set<Value*> &vs2);
    std::set<BasicBlock*> getInter(const std::set<BasicBlock*> &bs1,
            const std::set<BasicBlock*> &bs2);

    // Helper functions
    bool isEntryBlock(BasicBlock *BB);
    bool isPredOf(BasicBlock *child, BasicBlock *Parent);
    void reset();
    std::string getNewName(Value *allocainst);

    void performLiveAnalysis();
    void initDomSets();
    bool runIteration(); 

    BasicBlock *getIDom(BasicBlock *BB);
    void buildDomTree();

    void getDomFrontiers();

    std::set<BasicBlock*> computeIDF(std::vector<BasicBlock*> &defSites);
    std::vector<BasicBlock *> getDefSites(Value *allocainst);
    std::map<BasicBlock *, StoreInst *> getBlockDefs(AllocaInst *allocainst);

    void PlacePHINodes();
    void renamePass();
    void getPromAllocas();
    void rename(BasicBlock *);

  public:
    void run(Func *F);
};

bool isAllocaPromotable(AllocaInst *inst);

#endif

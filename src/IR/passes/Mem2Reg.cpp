#include "IR/passes/Mem2Reg.hpp"

#include <algorithm>
#include <cstddef>

using BlockVec = std::vector<BasicBlock *>;
using BlockSet = std::set<BasicBlock *>;
using ValSet = std::set<Value *>;
using domMap = std::map<BasicBlock *, BlockSet>;

using size_t = std::size_t;

//---

bool isAllocaPromotable(AllocaInst *inst) {
    TypeKind *allocatedType = inst->getValType();
    if (isArrayType(allocatedType)) return false;

    for (auto &use: inst->uses()) {
        Inst *useInst = use->getInst();
        unsigned no = use->getOpNo();

        if (auto load = dynamic_cast<LoadInst*>(useInst)) {
            if (load->getType() != allocatedType) return false;
        } else if (auto store = dynamic_cast<StoreInst*>(useInst)) {
            if (no != 1 || store->getValue()->getType() != allocatedType) return false;
        } else {
            return false;
        }
    }

    return true;
}

//---

void Mem2Reg::run(Func *func) {
    for (auto &block : func->getBlocks()) {
        if (!isEntryBlock(block.get()) && block->hasNPreds(0))
            continue;

        blockList.insert(block.get());
        blockVecList.push_back(block.get());
    }

    getPromAllocas();
    performLiveAnalysis();
    initDomSets();

    while (!runIteration())
        ;

    for (BasicBlock *BB : blockList)
        iDoms[BB] = getIDom(BB);

    buildDomTree();
    getDomFrontiers();
    PlacePHINodes();
    renamePass();

    reset();
}

bool Mem2Reg::isEntryBlock(BasicBlock *BB) {
    return BB == (BB->getParent()->getEntryBlock());
}

void Mem2Reg::initDomSets() {
    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB)) {
            BlockSet selfContList = {BB};
            domSets[BB] = selfContList;
        } else {
            BlockSet allNodeList = blockList;
            domSets[BB] = allNodeList;
        }
    }
}

ValSet Mem2Reg::getDiff(const ValSet &vs1, const ValSet &vs2) {
    ValSet result;

    std::set_difference(vs1.begin(), vs1.end(), vs2.begin(), vs2.end(),
                        std::inserter(result, result.begin()));

    return result;
}

ValSet Mem2Reg::getUnion(const ValSet &vs1, const ValSet &vs2) {
    ValSet result;

    std::set_union(vs1.begin(), vs1.end(), vs2.begin(), vs2.end(),
                   std::inserter(result, result.begin()));

    return result;
}

BlockSet Mem2Reg::getInter(const BlockSet &bs1, const BlockSet &bs2) {
    BlockSet result;

    std::set_intersection(bs1.begin(), bs1.end(), bs2.begin(), bs2.end(),
                          std::inserter(result, result.begin()));

    return result;
}

bool Mem2Reg::runIteration() {
    int changes = 0;

    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB))
            continue;

        BlockSet initialList = blockList;

        for (BasicBlock *Pred : BB->getPredecessors()) {
            BlockSet predDomSet = domSets[Pred];

            initialList = getInter(initialList, predDomSet);
        }

        initialList.insert(BB);

        if (domSets[BB] != initialList) {
            changes++;
            domSets[BB] = initialList;
        }
    }

    return changes == 0;
}

BasicBlock *Mem2Reg::getIDom(BasicBlock *BB) {
    BlockSet strictDomSet = domSets[BB];
    strictDomSet.erase(BB);

    BlockVec strictDomVec(strictDomSet.begin(), strictDomSet.end());

    for (size_t i = 0; i < strictDomVec.size(); i++) {

        bool allBlocksFound = true;
        BlockSet currBlockList = domSets[strictDomVec[i]];

        for (size_t j = 0; j < strictDomVec.size(); j++) {
            if (i == j) continue;
            if (currBlockList.count(strictDomVec[j]) == 0) allBlocksFound = false;
        }

        if (allBlocksFound) return strictDomVec[i];
    }

    return nullptr;
}

void Mem2Reg::buildDomTree() {
    for (BasicBlock *BB : blockList) {
        BlockSet valSet;

        for (BasicBlock *BBInner : blockList) {
            if (iDoms[BBInner] == BB) valSet.insert(BBInner);
        }

        domTree[BB] = valSet;
    }
}

void Mem2Reg::getDomFrontiers() {
    for (BasicBlock *BB : blockList) {
        if (isEntryBlock(BB) || !BB->hasNPredsOrMore(2))
            continue;

        for (BasicBlock *Pred : BB->getPredecessors()) {
            BasicBlock *currentNode = Pred;

            while (currentNode != iDoms[BB]) {
                domFrontier[currentNode].insert(BB);
                currentNode = iDoms[currentNode];
            }
        }
    }
}

BlockSet Mem2Reg::computeIDF(BlockVec &defSites) {
    BlockSet result;
    BlockVec workList = defSites;

    while (!workList.empty()) {
        BasicBlock *B = workList[workList.size() - 1];
        workList.pop_back();

        for (BasicBlock *frontier : domFrontier[B]) {
            if (result.count(frontier) == 0) {
                result.insert(frontier);
                workList.push_back(frontier);
            }
        }
    }

    return result;
}

BlockVec Mem2Reg::getDefSites(Value *allocainst) {
    BlockVec defsites;
    for (Inst *U : allocainst->users()) {
        StoreInst *SI = dynamic_cast<StoreInst*>(U);

        if (SI) defsites.push_back(SI->getParent());
    }

    return defsites;
}

std::map<BasicBlock *, StoreInst *> Mem2Reg::getBlockDefs(AllocaInst *allocainst) {
    std::map<BasicBlock *, StoreInst *> blockDefs;

    for (Inst *U : allocainst->users()) {
        if (StoreInst *SI = dynamic_cast<StoreInst*>(U)) blockDefs[SI->getParent()] = SI;
    }

    return blockDefs;
}

void Mem2Reg::PlacePHINodes() {
    for (Value *value : promotableAllocas) {
        BlockVec defsites = getDefSites(value);

        BlockSet idfSites = computeIDF(defsites);

        AllocaInst *allocainst = dynamic_cast<AllocaInst*>(value);

        for (BasicBlock *idfBlock : idfSites) {
            if (!LiveInMap[idfBlock].count(value))
                continue;

            int num = idfBlock->getNumPreds();

            PHINode *phi = PHINode::Create(allocainst->getValType(), num,
                                           allocainst->getName(), idfBlock);
            
            valPhiPos[value].insert(phi);
            idfBlock->addPHI(phi);
        }
    }
}

bool Mem2Reg::isPredOf(BasicBlock *child, BasicBlock *Parent) {
    for (BasicBlock *Pred : child->getPredecessors()) {
        if (Pred == Parent)
            return true;
    }

    return false;
}

void Mem2Reg::getPromAllocas() {
    for (BasicBlock *BB : blockList) {
        if (!isEntryBlock(BB))
            continue;

        for (auto &I : BB->getInsts()) {
            AllocaInst *allInst = dynamic_cast<AllocaInst*>(I.get());

            if (allInst && isAllocaPromotable(allInst)) {
                promotableAllocas.insert(allInst);
            }
        }
    }
}

void Mem2Reg::performLiveAnalysis() {
    // Fill DefMap and UseMap
    for (BasicBlock *BB : blockList) {
        std::set<Value *> seenDefs;
        auto &insts = BB->getInsts();

        for (auto it = insts.begin(); it != insts.end();) {
            Inst *I = (*it++).get();

            LoadInst *loadinst = dynamic_cast<LoadInst*>(I);
            StoreInst *storeinst = dynamic_cast<StoreInst*>(I);

            if (storeinst) {
                Value *val = storeinst->getOperand(1);

                if (!promotableAllocas.count(val))
                    continue;

                seenDefs.insert(val);
                DefMap[BB].insert(val);
            }

            if (loadinst) {
                Value *val = loadinst->getOperand(0);

                if (!promotableAllocas.count(val))
                    continue;

                if (!seenDefs.count(val))
                    UseMap[BB].insert(val);
            }
        }
    }

    // Update LiveInMap and LiveOutMap until fixed point
    bool constant = false;
    while (!constant) {
        constant = true;

        for (BasicBlock *BB : blockVecList) {
            ValSet liveInSet = getUnion(UseMap[BB], getDiff(LiveOutMap[BB], DefMap[BB]));

            if (LiveInMap[BB] != liveInSet) {
                constant = false;
                LiveInMap[BB] = liveInSet;
            }

            ValSet liveOutSet;

            for (BasicBlock *succ : BB->getSuccessors()) {
                liveOutSet = getUnion(liveOutSet, LiveInMap[succ]);
            }

            if (liveOutSet != LiveOutMap[BB]) {
                constant = false;
                LiveOutMap[BB] = liveOutSet;
            }
        }
    }
}

std::string Mem2Reg::getNewName(Value *allocainst) {
    auto combine = [](std::string old, int subscript) {
        std::string newString;
        newString = old + "_" + std::to_string(subscript);
        return newString;
    };

    int i = counter[allocainst];
    counter[allocainst] += 1;

    return combine(allocainst->getName(), i);
}

void Mem2Reg::renamePass() {
    for (BasicBlock *BB : blockList) {
        if (!isEntryBlock(BB))
            continue;

        for (auto &I : BB->getInsts()) {
            AllocaInst *allInst = dynamic_cast<AllocaInst*>(I.get());

            if (allInst && isAllocaPromotable(allInst)) {
                counter[allInst] = 0;
            }
        }
    }

    rename(blockVecList[0]);
}

void Mem2Reg::rename(BasicBlock *BB) {
    auto &insts = BB->getInsts();
    std::vector<Inst*> erasables;
    std::vector<Inst*> allocasToErase;
        
    for (PHINode *phiNode : BB->phis()) {
        Value *allocainst = nullptr;
        
        for (auto &[value, nodes] : valPhiPos)
            if (nodes.count(phiNode)) allocainst = value;

        phiNode->setName(getNewName(allocainst));
        allocaValStack[allocainst].push(phiNode);
    }

    for (auto it = insts.begin(); it != insts.end();) {
        Inst *I = (*it++).get();
        LoadInst *loadinst = dynamic_cast<LoadInst*>(I);
        StoreInst *storeinst = dynamic_cast<StoreInst*>(I);

        if (storeinst) {
            Value *storedVal = storeinst->getOperand(0);
            Value *ptrVal = storeinst->getOperand(1);

            allocaValStack[ptrVal].push(storedVal);
        }

        if (loadinst) {
            Value *ptrVal = loadinst->getOperand(0);

            if (!promotableAllocas.count(ptrVal)) continue;

            Value *currVal = allocaValStack[ptrVal].top();

            I->replaceAllUsesWith(currVal);
            erasables.push_back(I);
        }
    }

    for (BasicBlock *successor : BB->getSuccessors()) {
        for (PHINode *phiNode : successor->phis()) {
            Value *allocainst = nullptr;

            for (auto &[value, nodes] : valPhiPos) 
                if (nodes.count(phiNode)) allocainst = value;

            if (!allocaValStack.count(allocainst)) continue;

            Value *val = allocaValStack[allocainst].top();

            bool found = false;

            for (BasicBlock *phiBB : phiNode->blocks())
                if (phiBB == BB) found = true;

            if (!found) phiNode->addIncoming(val, BB);
        }
    }

    for (BasicBlock *child : domTree[BB])
        rename(child);

    for (auto it = insts.begin(); it != insts.end();) {
        Inst *I = (*it++).get();
        StoreInst *storeinst = dynamic_cast<StoreInst*>(I);
        AllocaInst *allocainst = dynamic_cast<AllocaInst*>(I);

        if (promotableAllocas.count(allocainst)) allocasToErase.push_back(I);

        if (storeinst) {
            Value *ptrVal = storeinst->getOperand(1);
            allocaValStack[ptrVal].pop();

            if (promotableAllocas.count(ptrVal)) erasables.push_back(I);
        }
    }

    for (PHINode *phiNode : BB->phis()) {
        Value *allocainst = nullptr;
        
        for (auto &[value, nodes] : valPhiPos)
            if (nodes.count(phiNode)) allocainst = value;
        
        allocaValStack[allocainst].pop();
    }

    for (Inst *inst: erasables) inst->eraseFromParent();
    for (Inst *inst: allocasToErase) inst->eraseFromParent();
}

void Mem2Reg::reset() {
    blockList.clear();
    domSets.clear();
    iDoms.clear();
    domTree.clear();
    domFrontier.clear();
    iDF.clear();

    blockVecList.clear();
    valPhiPos.clear();
    allocaValStack.clear();
    counter.clear();
    promotableAllocas.clear();

    UseMap.clear();
    DefMap.clear();
    LiveInMap.clear();
    LiveOutMap.clear();
}

#include "backend/riscv64/RegAlloc.hpp"
#include "backend/riscv64/mOperand.hpp"

#include <iostream>
#include <algorithm>
#include <stack>

using namespace RISCV;
using size_t = std::size_t;

void RegAlloc::allocate() {
    for (auto &mfunc : module->getFuncs()) {
        func = mfunc.get();
        run(func);
    }
}

void RegAlloc::run(mFunc *func) {
    reset();

    for (auto &bb : func->getBlocks())
        blockVec.push_back(bb.get());

    virLiveAnalysis();
    phyLiveAnalysis();
    createITFGraph();
    colour();
}

bool RegAlloc::isCalleeSaved(Reg reg) {
    return reg2Int[reg] >= CALLER_S;
}

void RegAlloc::addNode(VirtReg *vreg) {
    interference[vreg];
}

unsigned RegAlloc::getDegree(VirtReg *vreg) {
    return interference[vreg].size(); 
}

unsigned RegAlloc::getConflicts(VirtReg *vreg) {
    if (conflicts.count(vreg) == 0) return 0;
    return conflicts[vreg].count();
}

void RegAlloc::addEdge(VirtReg *v1, VirtReg *v2) {
    if (v1 == v2) return;

    interference[v1].insert(v2);
    interference[v2].insert(v1);
}

void RegAlloc::remEdge(VirtReg *v1, VirtReg *v2) {
    if (v1 == v2) return;
    if (!interference[v1].contains(v2)) return;

    interference[v1].erase(v2);
    interference[v2].erase(v1);
}

defUses RegAlloc::getDefUses(mInst *inst) {
    defUses result;

    unsigned numOpers = inst->getNumOperands();
    
    for (unsigned i = 0; i < numOpers; i++) {
        mOperand *oper = inst->getOperand(i);

        if (VirtReg *virtReg = dynamic_cast<VirtReg *>(oper)) {
            if (inst->isDef(i)) result.vDefs.insert(virtReg);
            if (inst->isUse(i)) result.vUses.insert(virtReg);

        } else if (PhyReg *phyReg = dynamic_cast<PhyReg *>(oper)) {
            Reg reg = phyReg->getReg();
            if (reg2Int.count(reg) == 0) continue;

            if (inst->isDef(i)) result.pDefs.insert(reg);
            if (inst->isUse(i)) result.pUses.insert(reg);
        }
    }

    if (mCallInst *callinst = dynamic_cast<mCallInst*>(inst)) {
        unsigned i, numArgs = callinst->getNumArgs();

        for (i = 0; i < numArgs && i < 8; i++) {
            Reg reg = static_cast<Reg>(10 + i);
            result.pUses.insert(reg);
        }

        for (i = 0; i < CALLER_S; i++) {
            Reg reg = availReg[i];
            result.pDefs.insert(reg);
        }
    }

    if (inst->getOpCode() == Code::RET) {
        result.pUses.insert(Reg::A0);
    } 

    return result;
}

std::set<VirtReg *> RegAlloc::getDiff(const std::set<VirtReg *> &s1,
                                      const std::set<VirtReg *> &s2) {
    std::set<VirtReg *> result;

    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.begin()));

    return result;
}

std::set<VirtReg *> RegAlloc::getUnion(const std::set<VirtReg *> &s1,
                                       const std::set<VirtReg *> &s2) {
    std::set<VirtReg *> result;

    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(),
                   std::inserter(result, result.begin()));

    return result;
}

std::optional<Reg> RegAlloc::getPhyReg(std::bitset<NUM_REGS> &regs) {
    for (int i = 0; i < NUM_REGS; i++) {
        if (regs[i]) return availReg[i];
    }

    return {};
}

RegAlloc::RegAlloc(mModule *module) :
    module(module) {
    availReg = {Reg::A0, Reg::A1, Reg::A2, Reg::A3, Reg::A4, Reg::A5, Reg::A6, Reg::A7,
        Reg::T0, Reg::T1, Reg::T2, Reg::T3, Reg::T4, Reg::T5, 
        Reg::S1, Reg::S2, Reg::S3, Reg::S4, Reg::S5, Reg::S6, Reg::S7, Reg::S8,
        Reg::S9, Reg::S10, Reg::S11};;

    for (int i = 0; i < NUM_REGS; i++)
        reg2Int.insert({availReg[i], i});
}

void RegAlloc::virLiveAnalysis() {
    for (mBlock *block : blockVec) {
        std::set<VirtReg *> seenDefs;

        for (auto &inst : block->getInsts()) {
            mInst *instRaw = inst.get();
            auto [vUses, vDefs, pUses, pDefs] = getDefUses(instRaw);

            for (VirtReg *virtreg: vUses) {
                if (!seenDefs.contains(virtreg)) useMap[block].insert(virtreg);
            }

            for (VirtReg *virtreg: vDefs) {
                seenDefs.insert(virtreg);
            }
        }

        defMap[block] = seenDefs;
    }

    bool constant = false;
    while (!constant) {
        constant = true;

        for (mBlock *block : blockVec) {
            std::set<VirtReg *> liveInSet =
                getUnion(useMap[block], getDiff(liveOutMap[block], defMap[block]));

            if (liveInMap[block] != liveInSet) {
                constant = false;
                liveInMap[block] = liveInSet;
            }

            std::set<VirtReg *> liveOutSet;

            for (mBlock *succ : block->getSuccs()) {
                liveOutSet = getUnion(liveOutSet, liveInMap[succ]);
            }

            if (liveOutSet != liveOutMap[block]) {
                constant = false;
                liveOutMap[block] = liveOutSet;
            }
        }
    }
}

void RegAlloc::phyLiveAnalysis() {
    for (mBlock *block : blockVec) {
        std::bitset<NUM_REGS> seenDefs;

        for (auto &inst : block->getInsts()) {
            mInst *instRaw = inst.get();
            auto [vUses, vDefs, pUses, pDefs] = getDefUses(instRaw);

            for (Reg useReg: pUses) {
                if (!seenDefs[reg2Int[useReg]]) puseMap[block].set(reg2Int[useReg]);
            }

            for (Reg defReg: pDefs) {
                seenDefs.set(reg2Int[defReg]);
            }
        }

        pdefMap[block] = seenDefs;
    }

    bool constant = false;
    while (!constant) {
        constant = true;

        for (mBlock *block : blockVec) {
            std::bitset<NUM_REGS> pliveInSet =
                puseMap[block] | (pliveOutMap[block] & ~pdefMap[block]);

            if (pliveInMap[block] != pliveInSet) {
                constant = false;
                pliveInMap[block] = pliveInSet;
            }

            std::bitset<NUM_REGS> pliveOutSet;

            for (mBlock *succ : block->getSuccs()) {
                pliveOutSet = pliveOutSet | pliveInMap[succ];
            }

            if (pliveOutSet != pliveOutMap[block]) {
                constant = false;
                pliveOutMap[block] = pliveOutSet;
            }
        }
    }
}


void RegAlloc::createITFGraph() {
    for (mBlock *block : blockVec) {
        auto &instructions = block->getInsts();

        std::set<VirtReg *> liveNow = liveOutMap[block];
        std::bitset<NUM_REGS> pliveNow = pliveOutMap[block];

        for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
            mInst *inst = (*it).get();
            
            auto [vUses, vDefs, pUses, pDefs] = getDefUses(inst);
            
            for (VirtReg *defReg : vDefs) {
                addNode(defReg);

                for (VirtReg *liveReg : liveNow) {
                    addNode(liveReg);
                    addEdge(defReg, liveReg);
                }

                for (unsigned i = 0; i < NUM_REGS; i++) {
                    if (pliveNow[i]) {
                        Reg reg = availReg[i];
                        conflicts[defReg].set(reg2Int[reg]);
                    } 
                }    
            } 

            for (Reg pdefReg: pDefs) {
                for (VirtReg *liveReg : liveNow)
                    conflicts[liveReg].set(reg2Int[pdefReg]);
            }
            
            for (Reg pdefReg: pDefs)
                pliveNow.reset(reg2Int[pdefReg]);

            for (Reg puseReg: pUses)
                pliveNow.set(reg2Int[puseReg]);

            for (VirtReg *defReg: vDefs)
                liveNow.erase(defReg);

            for (VirtReg *useReg: vUses)
                liveNow.insert(useReg); 
        }
    }

#ifdef DEBUG 
    printITFGraph(std::cout);
#endif
}

void RegAlloc::colour() {
    constexpr size_t k = availReg.size();
    workingITG = interference;
    std::stack<VirtReg *> vregStack;

    std::vector<VirtReg*> worklist; 

    for (auto it = interference.begin(); it != interference.end();) {
        auto &[vreg, _] = *(it++);
        
        if (getDegree(vreg) < k - getConflicts(vreg)) worklist.push_back(vreg);
    }

    while (!worklist.empty()) {
        VirtReg *node = worklist.back();
        worklist.pop_back();
        vregStack.push(node);

        auto neighbours = interference[node];

        for (auto vreg: neighbours) {
            remEdge(node, vreg);
            if (getDegree(vreg) < k - getConflicts(vreg)) worklist.push_back(vreg);
        }
            
        interference.erase(node);
    }

    if (!interference.empty()) {
        for (auto it = interference.begin(); it != interference.end();) {
            auto &[vreg, _] = *(it++);
            vregStack.push(vreg);
        }
    }

    std::bitset<NUM_REGS> availRegSet;
    availRegSet.flip();

    while (!vregStack.empty()) {
        VirtReg *curr = vregStack.top();
        vregStack.pop();
        std::bitset<NUM_REGS> adjRegs;

        if (regMap.contains(curr)) continue;

        for (auto &neighbour : workingITG[curr]) {
            if (regMap.contains(neighbour)) {
                Reg neighReg = regMap[neighbour];
                adjRegs.set(reg2Int[neighReg]);
            }
        }
        
        if (conflicts.count(curr) != 0) adjRegs = adjRegs | conflicts[curr];;
        
        std::bitset<NUM_REGS> validRegs = availRegSet & ~adjRegs;

        std::optional<Reg> optReg = getPhyReg(validRegs);
        
        if (optReg.has_value()) {
            Reg reg = regMap[curr] = optReg.value();
            if (isCalleeSaved(reg)) func->addCalleeSavedReg(reg);  
        } else {
            func->addSpill(curr);     
        }
    }

    replace(func);
    addSpills(func);
}

void RegAlloc::reset() {
    useMap.clear();
    defMap.clear();
    liveInMap.clear();
    liveOutMap.clear();

    puseMap.clear();
    pdefMap.clear();
    pliveInMap.clear();
    pliveOutMap.clear();

    interference.clear();
    workingITG.clear();
    conflicts.clear();
    
    regMap.clear();
    blockVec.clear();
}

void RegAlloc::replace(mFunc *func) {
    for (auto &bb : func->getBlocks()) {
        for (auto &inst : bb->getInsts()) {
            unsigned numOperands = inst->getNumOperands();

            for (unsigned i = 0; i < numOperands; i++) {
                mOperand *oper = inst->getOperand(i);

                if (auto vreg = dynamic_cast<VirtReg *>(oper)) {
                    if (!regMap.contains(vreg)) continue;

                    inst->setOperand(i, PhyReg::Create(regMap[vreg]));
                }
            }
        }
    }
}

void RegAlloc::addSpills(mFunc *func) {
    std::vector<mInst*> defs;
    std::vector<std::pair<mInst*, unsigned>> uses;

    auto getStore = [&](VirtReg *virtreg, mBlock *block) {
        TypeKind *type = virtreg->getType();
        std::size_t size = type == nullptr ? 8 : type->size;
        Code opc;

        switch (size) {
            case 1: opc = Code::SB;
            break;
            case 2: opc = Code::SH;
            break;
            case 4: opc = Code::SW;
            break;
            default: opc = Code::SD;
        }
        
        unsigned id = func->getSpillId(virtreg);
        std::vector<mOperand*> operands = {PhyReg::Create(Reg::T6), func->getStackSlot(id)};

        return std::make_unique<mInst>(opc, block, operands);
    };

    auto getLoad = [&](VirtReg *virtreg, mBlock *block) {
        TypeKind *type = virtreg->getType();
        std::size_t size = type == nullptr ? 8 : type->size;
        Code opc;

        switch (size) {
            case 1: opc = Code::LB;
            break;
            case 2: opc = Code::LH;
            break;
            case 4: opc = Code::LW;
            break;
            default: opc = Code::LD;
        }
        
        unsigned id = func->getSpillId(virtreg);
        std::vector<mOperand*> operands = {PhyReg::Create(Reg::T6), func->getStackSlot(id)};

        return std::make_unique<mInst>(opc, block, operands);
    };

    for (auto &block: func->getBlocks()) {
        for (auto &inst: block->getInsts()) {
            unsigned numOperands = inst->getNumOperands();

            for (unsigned i = 0; i < numOperands; i++) {
                VirtReg *vreg = dynamic_cast<VirtReg*>(inst->getOperand(i));
                if (vreg != nullptr) {
                    if (inst->isDef(i)) defs.push_back(inst.get()); 
                    if (inst->isUse(i)) uses.push_back(std::make_pair(inst.get(), i));
                }
            }
        }
    }

    for (auto &inst: defs) {
        unsigned i, numOperands = inst->getNumOperands();

        for (i = 0; i < numOperands; i++) {
            if (inst->isDef(i)) break;            
        }

        mOperand *oper = inst->getOperand(i);
        VirtReg *vreg = dynamic_cast<VirtReg*>(oper);
        if (vreg == nullptr) continue;

        inst->setOperand(i, PhyReg::Create(Reg::T6));
        mBlock *mblock = inst->getParent();

        auto sinst = getStore(vreg, mblock);
        mblock->insertAfter(std::move(sinst), inst);
    }

    for (auto &[inst, i]: uses) {
        mOperand *oper = inst->getOperand(i);
        VirtReg *vreg = dynamic_cast<VirtReg*>(oper);
        if (vreg == nullptr) continue;
        
        inst->setOperand(i, PhyReg::Create(Reg::T6));
        mBlock *mblock = inst->getParent();

        auto linst = getLoad(vreg, mblock);
        mblock->insertBefore(std::move(linst), inst);
    }
}

void RegAlloc::printLiveSets(std::ostream &os) {
    for (mBlock *block : blockVec) {
        os << std::format("{}\n", block->getName());

        os << "Uses: [";
        for (auto &virtReg : useMap[block])
            virtReg->print(os);
        os << "]\n";

        os << "Defs: [";
        for (auto &virtReg : defMap[block])
            virtReg->print(os);
        os << "]\n";

        os << "LiveIn: [";
        for (auto &virtReg : liveInMap[block])
            virtReg->print(os);
        os << "]\n";

        os << "LiveOut: [";
        for (auto &virtReg : liveOutMap[block])
            virtReg->print(os);
        os << "]\n\n";
    }
}

void RegAlloc::printITFGraph(std::ostream &os) {
    tabulate::Table graph;
    graph.add_row({"Node", "Edges"});

    unsigned i = 1;
    for (auto &[key, value] : interference) {
        std::string edges;

        for (auto &vreg : value) {
            edges += std::format("{} ", vreg->getPrintStr());
        }

        graph.add_row({key->getPrintStr(), edges});
        i++;
    }

    os << graph << "\n";
}

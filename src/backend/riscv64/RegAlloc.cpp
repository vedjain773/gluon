#include "backend/riscv64/RegAlloc.hpp"
#include "backend/riscv64/mOperand.hpp"

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
    blockVec.clear();

    for (auto &bb : func->getBlocks()) {
        blockVec.push_back(bb.get());
    }

    performLiveAnalysis();
    createITFGraph();
    colour();
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

std::set<VirtReg *> RegAlloc::getDiff(const std::set<VirtReg *> &s1,
                                      const std::set<VirtReg *> &s2) {
    std::set<VirtReg *> result;

    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.begin()));

    return result;
}

std::set<VirtReg *> RegAlloc::getInter(const std::set<VirtReg *> &s1,
                                       const std::set<VirtReg *> &s2) {
    std::set<VirtReg *> result;

    std::set_intersection(s1.begin(), s1.end(), s2.begin(), s2.end(),
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

std::set<Reg> RegAlloc::getRegDiff(const std::set<Reg> &s1, const std::set<Reg> &s2) {
    std::set<Reg> result;

    std::set_difference(s1.begin(), s1.end(), s2.begin(), s2.end(),
                        std::inserter(result, result.begin()));

    return result;
}

RegAlloc::RegAlloc(mModule *module) :
    module(module) {
    availReg = {Reg::T0, Reg::T1, Reg::T2, Reg::T3, Reg::T4, Reg::T5, Reg::T6};
}

void RegAlloc::createITFGraph() {
    for (mBlock *block : blockVec) {
        std::vector<std::unique_ptr<mInst>> &instructions = block->getInsts();
        std::set<VirtReg *> liveNow = liveOutMap[block];

        for (auto it = instructions.rbegin(); it != instructions.rend(); ++it) {
            mInst *inst = (*it).get();
            std::vector<VirtReg *> defs;
            std::vector<VirtReg *> uses;

            for (unsigned i = 0; i < inst->getNumOperands(); i++) {
                VirtReg *virtReg = dynamic_cast<VirtReg *>(inst->getOperand(i));

                if (virtReg == nullptr) continue;
                interference[virtReg];

                if (inst->isDef(i)) defs.push_back(virtReg);
                else if (inst->isUse(i)) uses.push_back(virtReg);
            }

            for (VirtReg *defReg : defs) {
                for (VirtReg *liveReg : liveNow) {
                    addEdge(defReg, liveReg);
                }
            }

            for (VirtReg *defReg : defs)
                liveNow.erase(defReg);
            for (VirtReg *useReg : uses)
                liveNow.insert(useReg);
        }
    }
}

void RegAlloc::performLiveAnalysis() {
    for (mBlock *block : blockVec) {
        std::set<VirtReg *> seenDefs;

        for (auto &inst : block->getInsts()) {
            mInst *instRaw = inst.get();
            unsigned size = instRaw->getNumOperands();

            if (size == 0) continue;

            for (size_t i = 0; i < size; i++) {
                mOperand *reg = instRaw->getOperand(i);
                VirtReg *virtReg = dynamic_cast<VirtReg *>(reg);

                if (virtReg == nullptr) continue;
                if (instRaw->isUse(i) && !seenDefs.contains(virtReg)) {
                    useMap[block].insert(virtReg);
                }

                if (instRaw->isDef(i)) seenDefs.insert(virtReg);
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

void RegAlloc::colour() {
    size_t k = availReg.size();
    workingITG = interference;
    std::stack<VirtReg *> vregStack;

    while (!interference.empty()) {
        VirtReg *node = nullptr;
        bool found = false;

        for (auto it = interference.begin(); it != interference.end();) {
            auto &[vreg, vregSet] = *(it++);

            if (vregSet.size() < k) {
                found = true;
                node = vreg;
            }
        }

        if (found) {
            vregStack.push(node);
            for (auto &vreg: workingITG[node]) remEdge(node, vreg);
            
            interference.erase(node);
        } else {
            break;
        }
    }

    std::set<Reg> availRegSet(availReg.begin(), availReg.end());

    while (!vregStack.empty()) {
        VirtReg *curr = vregStack.top();
        vregStack.pop();
        std::set<Reg> adjRegs;

        if (regMap.contains(curr)) continue;

        for (auto &neighbour : workingITG[curr]) {
            if (regMap.contains(neighbour)) adjRegs.insert(regMap[neighbour]);
        }

        std::set<Reg> validRegs = getRegDiff(availRegSet, adjRegs);
        if (validRegs.empty()) continue;

        regMap[curr] = *(validRegs.begin());
    }

    replace(func);
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
    for (auto &[key, value] : workingITG) {
        std::string edges;

        for (auto &vreg : value) {
            edges += std::format("{} ", vreg->getPrintStr());
        }

        graph.add_row({key->getPrintStr(), edges});
        i++;
    }

    os << graph << "\n";
}

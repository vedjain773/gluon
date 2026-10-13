#include "backend/riscv64/PHIPass.hpp"
#include <map>

using namespace RISCV;

PhiPass::PhiPass(mModule *mmod):
    mmod(mmod) {}

void PhiPass::addDef(std::pair<mOperand*, mBlock*> &pair, mOperand *oper) {
    auto &[op, block] = pair;
    OpKind opkind = op->getOpkind();

    std::vector<mOperand*> opers = {oper, op};
    Code opc;

    if (opkind == OpKind::VirtualReg) opc = Code::MV;
    else if (opkind == OpKind::Immediate) opc = Code::LI;
    else opc = Code::LW;

    auto inst = std::make_unique<mInst>(opc, block, opers);
    block->insertPnel(std::move(inst));
}

void PhiPass::insertDefs(mFunc *func) {
    std::vector<mInst*> phis;

    for (auto &mblock: func->getBlocks()) {
        for (auto &minst: mblock->getInsts()) {
            if (minst->getOpCode() == Code::P_PHI) phis.push_back(minst.get());
        }
    }

    for (auto &phi: phis) {
        mOperand *oper = phi->getOperand(0);
        mPhiInst *phiInst = dynamic_cast<mPhiInst*>(phi);
        std::vector<std::pair<mOperand*, mBlock*>> &pair = phiInst->getPairs();

        for (auto &pairEle: pair) addDef(pairEle, oper); 
    } 
}

void PhiPass::remPHI(mFunc *func) {
    std::map<mBlock*, std::vector<mInst*>> phiMap;

    for (auto &mblock: func->getBlocks()) {
        for (auto &minst: mblock->getInsts()) {
            if (minst->getOpCode() == Code::P_PHI) {
                mBlock *block = mblock.get();
                phiMap[block].push_back(minst.get());
            }
        }
    } 

    for (auto &[block, phis]: phiMap) {
        for (auto &phi: phis) {
            block->removeInst(phi);
        }
    }
}

void PhiPass::run() {
    for (auto &func : mmod->getFuncs()) {
        insertDefs(func.get());
        remPHI(func.get());
    }
}

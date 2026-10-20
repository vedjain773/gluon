#include "backend/riscv64/Peephole.hpp"

using namespace RISCV;

Peephole::Peephole(mModule *mod):
    mod(mod) {}

void Peephole::updateMod(mModule *mod) {
    this->mod = mod;
}

void Peephole::applyIdentity(mInst *inst) {
    Code opc = inst->getOpCode();

    switch (opc) {
        case Code::ADD: {
            mOperand *op1 = inst->getOperand(1);
            mOperand *op2 = inst->getOperand(2);

            if (PhyReg *phyreg = dynamic_cast<PhyReg*>(op1)) {
                if (phyreg->getReg() == Reg::ZERO) inst->remOperand(op1);
                inst->setOpCode(Code::MV);
            } else if (PhyReg *phyreg = dynamic_cast<PhyReg*>(op2)) {
               if (phyreg->getReg() == Reg::ZERO) inst->remOperand(op2);
               inst->setOpCode(Code::MV);
            }

        } break;

        case Code::ADDI: {
            mOperand *op1 = inst->getOperand(1);
            mOperand *op2 = inst->getOperand(2);

            if (Immediate *imm = dynamic_cast<Immediate*>(op1)) {
                if (imm->getImmValue() == 0) inst->remOperand(op1);
                inst->setOpCode(Code::MV);
            } else if (Immediate *imm = dynamic_cast<Immediate*>(op2)) {
                if (imm->getImmValue() == 0) inst->remOperand(op2);
                inst->setOpCode(Code::MV);
            }

        } break;

        case Code::SUB: {
            mOperand *op2 = inst->getOperand(2);

            if (PhyReg *phyreg = dynamic_cast<PhyReg*>(op2)) {
               if (phyreg->getReg() == Reg::ZERO) inst->remOperand(op2);
               inst->setOpCode(Code::MV);
            }

        } break;

        case Code::SLT:
        case Code::SGT: {
            mOperand *op1 = inst->getOperand(1);
            mOperand *op2 = inst->getOperand(2);

            if (op1 == op2) {
                inst->remOperand(op1);
                inst->remOperand(op2);
                
                inst->setOpCode(Code::LI);
            }
        } break;


        default: return;
    }
}

void Peephole::redInst() {
    for (auto &func: mod->getFuncs()) {
        for (auto &block: func->getBlocks()) {
            for (auto &inst: block->getInsts()) {
                applyIdentity(inst.get()); 
            }
        } 
    } 
}

void Peephole::remJump() {
    for (auto &func: mod->getFuncs()) {
        auto &blocks = func->getBlocks();

        for (auto it = blocks.begin(); it != blocks.end() - 1;) {
            mBlock *block = (*it++).get();
            if (block->getNumInsts() == 0) continue;

            mInst *inst = block->getTerminator();

            Code opc = inst->getOpCode();
            if (opc != Code::J) continue;

            mBlock *next = (*it).get();
            mBrInst *mbrinst = dynamic_cast<mBrInst*>(inst);

            if (mbrinst == nullptr) continue;
            mBlock *label = mbrinst->getBlock();
            
            if (label == next) inst->eraseFromParent();
        }
    } 
}

void Peephole::remMoves() {
    for (auto &func: mod->getFuncs()) {
        for (auto &block: func->getBlocks()) {
            auto &insts = block->getInsts();

            if (insts.empty()) continue;
            std::vector<mInst*> erasables;

            for (auto it = insts.begin(); it != insts.end(); ++it) {
                mInst *curr = it->get();
                Code opc = curr->getOpCode();

                if (opc != Code::MV) continue;

                PhyReg *curr0 = dynamic_cast<PhyReg*>(curr->getOperand(0));
                PhyReg *curr1 = dynamic_cast<PhyReg*>(curr->getOperand(1));
                
                if (curr0->getReg() == curr1->getReg())
                    erasables.push_back(curr);
            }

            for (mInst *inst: erasables)
                inst->eraseFromParent();
        } 
    } 
}

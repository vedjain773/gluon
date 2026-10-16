#include "backend/riscv64/Peephole.hpp"

using namespace RISCV;

Peephole::Peephole(mModule *mod):
    mod(mod) {}

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

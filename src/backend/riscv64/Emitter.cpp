#include "backend/riscv64/Emitter.hpp"
#include <fstream>

using namespace RISCV;

Emitter::Emitter(mModule *module, std::ostream &os)
    :module(module), os(os) {}

void Emitter::emit() {
    os << ".text\n"; 

    for (auto &func: module->getFuncs()) emitFunc(func.get()); 
}

void Emitter::emitFunc(mFunc *func) {
    os << std::format(".globl {0}\n{0}:\n", func->getName());

    for (auto &block: func->getBlocks()) emitBlock(block.get());
}

void Emitter::emitBlock(mBlock *block) {
    for (auto &inst: block->getInsts()) {
        os << "  ";
        emitInst(inst.get());
        os << '\n';
    }
}

void Emitter::emitInst(mInst *inst) {
    switch (inst->getOpCode()) {
        case Code::LI: emitLI(inst->getOperand(0), inst->getOperand(1));
        break;

        case Code::RET: os << std::format("{}\n", "ret"); 
        break;

        default: return;
    } 
}

void Emitter::emitLI(mOperand *reg, mOperand *imm) {
    os << std::format("li ");
    emitOper(reg);
    os << ", ";
    emitOper(imm);
}

void Emitter::emitOper(mOperand *oper) {
    switch (oper->getOpkind()) {
        case OpKind::Immediate: {
            Immediate *imm = dynamic_cast<Immediate*>(oper);
            os << imm->getImmValue();
        } break;

        case OpKind::PhysicalReg: {
            PhyReg *phy = dynamic_cast<PhyReg*>(oper);
            os << regToStr(phy->getReg());
        } break;

        default: return;
    }
}

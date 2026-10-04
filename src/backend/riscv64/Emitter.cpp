#include "backend/riscv64/Emitter.hpp"
#include <fstream>

using namespace RISCV;

constexpr std::array<std::string, 28> codeNames = {
    "neg",  "addi", "add",  "sub",  "mul", "div", "rem", "sgt", "slt", "seqz",
    "snez", "mv",   "li",   "lb",   "lh",  "la",  "lw",  "ld",  "sb",  "sh",
    "sw",   "sd",   "beqz", "bnez", "ret", "j",   "nop", "p_la"};

Emitter::Emitter(mModule *module, std::ostream &os) :
    module(module),
    os(os) {}

unsigned Emitter::getId(mBlock *block) {
    unsigned id, numBlocks = blockIDs.size();
    for (id = 0; id < numBlocks; id++) {
        if (blockIDs[id] == block) return id;
    }

    return -1;
}

std::string Emitter::getInstStr(const Code &opcode) {
    return codeNames[static_cast<unsigned>(opcode)];
}

void Emitter::emit() {
    os << ".text\n";

    for (auto &func : module->getFuncs())
        emitFunc(func.get());
}

void Emitter::emitFunc(mFunc *func) {
    os << std::format(".globl {0}\n{0}:\n", func->getName());

    blockIDs.clear();
    for (auto &block : func->getBlocks())
        blockIDs.push_back(block.get());

    for (auto &block : func->getBlocks())
        emitBlock(block.get());
}

void Emitter::emitBlock(mBlock *block) {
    os << std::format(".LBB{}:\n", getId(block));

    for (auto &inst : block->getInsts()) {
        os << "  ";
        emitInst(inst.get());
        os << "\n";
    }

    os << '\n';
}

void Emitter::emitInst(mInst *inst) {
    os << std::format("{} ", getInstStr(inst->getOpCode()));
    Code opc = inst->getOpCode();

    unsigned numOpers = inst->getNumOperands();
    for (unsigned i = 0; i < numOpers; i++) {
        emitOper(inst->getOperand(i));

        if (i != numOpers - 1) os << ", ";
    }

    mBrInst *mbrinst = dynamic_cast<mBrInst *>(inst);
    if (mbrinst != nullptr) {
        if (opc != Code::J) os << ", ";

        os << std::format(".LBB{}", getId(mbrinst->getBlock()));
    }
}

void Emitter::emitOper(mOperand *oper) {
    if (oper == nullptr) return;

    switch (oper->getOpkind()) {
        case OpKind::Immediate: {
            Immediate *imm = dynamic_cast<Immediate *>(oper);
            os << imm->getImmValue();
        } break;

        case OpKind::MemOperand: {
            MemOperand *mem = dynamic_cast<MemOperand *>(oper);
            os << std::format("{}({})", mem->getOffset(), regToStr(mem->getBase()));
        } break;

        case OpKind::PhysicalReg: {
            PhyReg *phy = dynamic_cast<PhyReg *>(oper);
            os << regToStr(phy->getReg());
        } break;

        default:
            return;
    }
}

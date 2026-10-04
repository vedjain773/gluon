#include "backend/riscv64/mInst.hpp"
#include "backend/riscv64/mBlock.hpp"
#include <format>

using namespace RISCV;

constexpr std::array<std::string, 28> codeNames = {
    "NEG",
    "ADDI", "ADD", "SUB", "MUL", "DIV", "REM",
    "SGT", "SLT", "SEQZ", "SNEZ",
    "MV",
    "LI", "LB", "LH", "LA", "LW", "LD",
    "SB", "SH", "SW", "SD",
    "BEQZ", "BNEZ", "RET", "J",
    "NOP",
    "P_LA"
};

std::string RISCV::codeToStr(const Code &code) {
    return codeNames[static_cast<unsigned>(code)]; 
}

mInst::mInst(Code opcode, mBlock *parent, std::vector<mOperand*> operands)
    :opcode(opcode), parent(parent), operands(operands) {}

Code mInst::getOpCode() { return opcode; }

mBlock *mInst::getParent() { return parent; }

unsigned mInst::getNumOperands() { return operands.size(); }

mOperand *mInst::getOperand(unsigned i) { return operands[i]; }

void mInst::setOperand(unsigned i, mOperand *operand) {
    operands[i] = operand;
}

bool mInst::isDef(unsigned i) {
    switch (opcode) {
        case Code::ADD:
        case Code::SUB:
        case Code::MUL:
        case Code::DIV:
        case Code::REM:
        case Code::SGT:
        case Code::SLT:
            return i == 0;

        case Code::NEG:
        case Code::SEQZ:
        case Code::SNEZ:
        case Code::MV:
        case Code::LI:
        case Code::LB:
        case Code::LH:
        case Code::LA:
        case Code::LW:
        case Code::LD:
        case Code::P_LA:
            return i == 0;

        default:
            return false;
    }
}

bool mInst::isUse(unsigned i) {
    switch (opcode) {
        case Code::ADD:
        case Code::SUB:
        case Code::MUL:
        case Code::DIV:
        case Code::REM:
        case Code::SGT:
        case Code::SLT:
            return i == 1 || i == 2;

        case Code::NEG:
        case Code::SEQZ:
        case Code::SNEZ:
        case Code::MV:
            return i == 1;

        case Code::LI:
            return i == 1;

        case Code::LB:
        case Code::LH:
        case Code::LA:
        case Code::LW:
        case Code::LD:
        case Code::P_LA:
            return i == 1;

        case Code::SB:
        case Code::SH:
        case Code::SW:
        case Code::SD:
            return i == 0 || i == 1;

        case Code::BEQZ:
        case Code::BNEZ:
            return i == 0;

        case Code::RET:
            return i == 0;

        default:
            return false;
    }
}

void mInst::print(std::ostream &os) {
    os << std::format("{} ", codeToStr(opcode));

    for (auto &oper: operands) oper->print(os);
}

//---

mBrInst::mBrInst(Code opcode, mBlock *parent, mOperand *cond, mBlock *label)
    :mInst(opcode, parent, {cond}), label(label) {}

void mBrInst::print(std::ostream &os) {
    os << std::format("{} ", codeToStr(getOpCode()));
   
    if (getNumOperands() != 0) {
        if (getOperand(0) != nullptr) getOperand(0)->print(os);
    }

    os << std::format(".L{}", label->getName());
}

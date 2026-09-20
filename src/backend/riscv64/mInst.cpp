#include "backend/riscv64/mInst.hpp"
#include "backend/riscv64/mBlock.hpp"
#include <format>

using namespace RISCV;

constexpr std::array<std::string, 26> codeNames = {
    "NEG",
    "ADD", "SUB", "MUL", "DIV", "REM",
    "SGT", "SLT", "SEQZ", "SNEZ",
    "MV",
    "LI", "LB", "LH", "LA", "LW", "LD",
    "SB", "SH", "SW", "SD",
    "BEQZ", "BNEZ", "RET", "J",
    "NOP"
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

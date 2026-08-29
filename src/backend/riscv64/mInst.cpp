#include "backend/riscv64/mInst.hpp"
#include <format>

using namespace RISCV;

std::string RISCV::codeToStr(const Code &code) {
    switch (code) {
        case Code::LI: return "LI";
        case Code::RET: return "RET";
        default: return "";
    }
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

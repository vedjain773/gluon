#include "backend/riscv64/mInst.hpp"

using namespace RISCV;

mInst::mInst(OpCode opcode, mBlock *parent, std::vector<mOperand> operands)
    :opcode(opcode), parent(parent), operands(operands) {}

OpCode mInst::getOpCode() { return opcode; }

mBlock *mInst::getParent() { return parent; }

unsigned mInst::getNumOperands() { return operands.size(); }

mOperand& mInst::getOperand(unsigned i) { return operands[i]; }

void mInst::setOperand(unsigned i, mOperand operand) {
    operands[i] = operand;
}

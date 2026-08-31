#include "backend/riscv64/mOperand.hpp"
#include <array>

using namespace RISCV;

constexpr std::array<std::string, 32> regNames {
    "zero", "ra", "sp", "gp", "tp",
    "t0", "t1", "t2",
    "s0", "s1",
    "a0", "a1", "a2", "a3",
    "a4", "a5", "a6", "a7",
    "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11",
    "t3", "t4", "t5", "t6"
};

mOperand::mOperand(const OpKind &opkind)
    :kind(opkind) {}

//---

Immediate::Immediate(int64_t value)
    :mOperand(OpKind::Immediate), value(value) {}

Immediate *Immediate::Create(int64_t value) {
    return new Immediate(value);
} 

int64_t Immediate::getImmValue() { return value; }

void Immediate::print(std::ostream &os) {
    os << std::format("{} ", value);
}

//---

VirtReg::VirtReg(unsigned no)
    :mOperand(OpKind::VirtualReg), no(no) {}

VirtReg *VirtReg::Create(unsigned no) {
    return new VirtReg(no);
}

unsigned VirtReg::getVirtRegNo() { return no; }

void VirtReg::print(std::ostream &os) {
    os << std::format("v{} ", no);
}

//---

PhyReg::PhyReg(Reg reg)
    :mOperand(OpKind::PhysicalReg), reg(reg) {}

PhyReg *PhyReg::Create(Reg reg) {
    return new PhyReg(reg);
}

Reg PhyReg::getReg() { return reg; }

void PhyReg::print(std::ostream &os) {
    os << regToStr(reg);
}

//---

std::string RISCV::regToStr(Reg reg) {
    return regNames[static_cast<unsigned>(reg)];
}

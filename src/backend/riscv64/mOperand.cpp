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

mOperand::mOperand(const OpKind &opkind, TypeKind *opType)
    :kind(opkind), opType(opType) {}

//---

VirtReg::VirtReg(unsigned no, TypeKind *regType)
    :mOperand(OpKind::VirtualReg, regType), no(no) {}

VirtReg *VirtReg::Create(unsigned no, TypeKind *regType) {
    return new VirtReg(no, regType);
}

unsigned VirtReg::getVirtRegNo() { return no; }

void VirtReg::print(std::ostream &os) {
    os << std::format("v{} ", no);
}

//---

StackSlot::StackSlot(unsigned id, TypeKind *stype)
    :mOperand(OpKind::StackSlot, stype), id(id) {}

StackSlot *StackSlot::Create(unsigned id, TypeKind *stype) {
    return new StackSlot(id, stype);
}

unsigned StackSlot::getSlotId() { return id; }

unsigned StackSlot::getSlotSize() { return opType->size; }

void StackSlot::print(std::ostream &os) {
    os << std::format("ss{} ", id);
}

//---

PhyReg::PhyReg(Reg reg)
    :mOperand(OpKind::PhysicalReg), reg(reg) {}

PhyReg *PhyReg::Create(Reg reg) {
    return new PhyReg(reg);
}

Reg PhyReg::getReg() { return reg; }

void PhyReg::print(std::ostream &os) {
    os << std::format("{} ", regToStr(reg));
}

//---

Immediate::Immediate(int64_t value, TypeKind *itype)
    :mOperand(OpKind::Immediate, itype), value(value) {}

Immediate *Immediate::Create(int64_t value, TypeKind *itype) {
    return new Immediate(value, itype);
} 

int64_t Immediate::getImmValue() { return value; }

void Immediate::print(std::ostream &os) {
    os << std::format("{} ", value);
}

//---

std::string RISCV::regToStr(Reg reg) {
    return regNames[static_cast<unsigned>(reg)];
}

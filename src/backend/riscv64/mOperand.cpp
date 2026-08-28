#include "backend/riscv64/mOperand.hpp"

using namespace RISCV;

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

PhyReg::PhyReg(unsigned no)
    :mOperand(OpKind::PhysicalReg), no(no) {}

PhyReg *PhyReg::Create(unsigned no) {
    return new PhyReg(no);
}

unsigned PhyReg::getPhyRegNo() { return no; }

void PhyReg::print(std::ostream &os) {
    os << std::format("x{} ", no);
}

//---

#ifndef MOPERAND_H
#define MOPERAND_H

#include "utils/Scope.hpp"
#include <cstdint>
#include <format>

using int64_t = std::int64_t;

namespace RISCV {

enum class OpKind {
    VirtualReg,
    MemOperand,
    PhysicalReg,
    Immediate
};

enum class Code: unsigned {
    NEG,
    ADDI, ADD, SUB, MUL, DIV, REM,
    SGT, SLT, SEQZ, SNEZ,
    MV,
    LI, LB, LH, LA, LW, LD,
    SB, SH, SW, SD,
    BEQZ, BNEZ, RET, J,
    CALL,
    NOP,
    P_LA
};

enum class Reg: unsigned {
    ZERO, RA, SP, GP, TP,
    T0, T1, T2, S0, S1,
    A0, A1, A2, A3, A4, A5, A6, A7,
    S2, S3, S4, S5, S6, S7, S8, S9, s10, S11,
    T3, T4, T5, T6
};

std::string regToStr(Reg reg);

class mOperand {
  private:
    OpKind kind;

  protected:
    TypeKind *opType;
    mOperand(const OpKind &opkind, TypeKind *opType = nullptr);

  public:
    OpKind getOpkind() { return kind; }
    TypeKind *getType() { return opType; }
   
    virtual void print(std::ostream &os) = 0;
    virtual ~mOperand() = default;
};

class VirtReg: public mOperand {
  private:
    unsigned no;
    VirtReg(unsigned no, TypeKind *regType);
  
  public:
    static VirtReg *Create(unsigned no, TypeKind *regType);
    unsigned getVirtRegNo();

    std::string getPrintStr();
    void print(std::ostream &os);
};

class MemOperand: public mOperand {
  private:
    int offset = 0;
    Reg base;

  public:
    MemOperand(TypeKind *sType, const Reg &base, int offset);
    
    static MemOperand *Create(TypeKind *sType, const Reg &base, int offset = 0);
    Reg getBase();
    int getOffset();

    void setOffset(int offset);
    void print(std::ostream &os);
};

class StackSlot: public MemOperand {
  private:
    unsigned id;
    StackSlot(unsigned id, TypeKind *stype);

  public:
    static StackSlot *Create(unsigned id, TypeKind *stype);
    unsigned getSlotId();
    unsigned getSlotSize();
};

class PhyReg: public mOperand {
  private:
    Reg reg; 
    PhyReg(Reg reg, TypeKind *type);
  
  public:
    static PhyReg *Create(Reg reg);
    static PhyReg *Create(Reg reg, TypeKind *type);
    Reg getReg();

    void print(std::ostream &os);
};

class Immediate: public mOperand {
  private:
    int64_t value;
    Immediate(int64_t value, TypeKind *itype);

  public:
    static Immediate *Create(int64_t value, TypeKind *itype);
    int64_t getImmValue();

    void print(std::ostream &os);
};

}

#endif

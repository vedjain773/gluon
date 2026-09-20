#ifndef MOPERAND_H
#define MOPERAND_H

#include <cstdint>
#include <format>

using int64_t = std::int64_t;

namespace RISCV {

enum class OpKind {
    VirtualReg,
    StackSlot,
    PhysicalReg,
    Immediate
};

enum class Code: unsigned {
    NEG,
    ADD, SUB, MUL, DIV, REM,
    SGT, SLT, SEQZ, SNEZ,
    MV,
    LI, LD,
    SW, SD,
    BEQZ, BNEZ, RET, J,
    NOP
};

enum class Reg: unsigned {
    ZERO, RA, SP, GP, TP,
    T0, T1, T2, SO, S1,
    A0, A1, A2, A3, A4, A5, A6, A7,
    S2, S3, S4, S5, S6, S7, S8, S9, S11,
    T3, T4, T5, T6
};

std::string regToStr(Reg reg);

class mOperand {
  private:
    OpKind kind;
    
  public:
    mOperand(const OpKind &opkind);
    OpKind getOpkind() { return kind; }
   
    virtual void print(std::ostream &os) = 0; 
    virtual ~mOperand() = default;
};

class VirtReg: public mOperand {
  private:
    unsigned no;
    VirtReg(unsigned no);
  
  public:
    static VirtReg *Create(unsigned no);
    unsigned getVirtRegNo();

    void print(std::ostream &os);
};

class StackSlot: public mOperand {
  private:
    unsigned size;
    unsigned id;
    StackSlot(unsigned size, unsigned id);

  public:
    static StackSlot *Create(unsigned size, unsigned id);
    unsigned getSlotId();
    unsigned getSlotSize();

    void print(std::ostream &os);
};

class PhyReg: public mOperand {
  private:
    Reg reg; 
    PhyReg(Reg reg);
  
  public:
    static PhyReg *Create(Reg reg);
    Reg getReg();

    void print(std::ostream &os);
};

class Immediate: public mOperand {
  private:
    int64_t value;
    Immediate(int64_t value);

  public:
    static Immediate *Create(int64_t value);
    int64_t getImmValue();

    void print(std::ostream &os);
};

}

#endif

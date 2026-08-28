#ifndef MOPERAND_H
#define MOPERAND_H

#include <cstdint>
#include <format>

using int64_t = std::int64_t;

namespace RISCV {

enum class OpKind {
    VirtualReg,
    PhysicalReg,
    Immediate
};

enum class Code {
    LI 
};

class mOperand {
  private:
    OpKind kind;
    
  public:
    mOperand(const OpKind &opkind);
    OpKind getOpkind() { return kind; }
   
    virtual void print(std::ostream &os) = 0; 
    virtual ~mOperand() = default;
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

class VirtReg: public mOperand {
  private:
    unsigned no;
    VirtReg(unsigned no);
  
  public:
    static VirtReg *Create(unsigned no);
    unsigned getVirtRegNo();

    void print(std::ostream &os);
};

class PhyReg: public mOperand {
  private:
    unsigned no;
    PhyReg(unsigned no);
  
  public:
    static PhyReg *Create(unsigned no);
    unsigned getPhyRegNo();

    void print(std::ostream &os);
};

}

#endif

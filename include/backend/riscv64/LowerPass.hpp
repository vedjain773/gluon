#ifndef LOWER_H
#define LOWER_H

#include "backend/mModule.hpp"
#include "IR/Module.hpp"

#include <unordered_map>
#include <initializer_list>

namespace RISCV {

class LowerPass {
  private:
    Module *module;

    mModule *mmod;
    mFunc *currFunc;
    mBlock *currBlock;

    std::unordered_map<BasicBlock*, mBlock*> blockMap;

    std::unordered_map<Value*, VirtReg*> virtualRegTable;
    std::unordered_map<Value*, StackSlot*> stackSlotTable;
    std::unordered_map<Reg, PhyReg*> phyregMap;

    unsigned currentRegNo;

    std::unordered_map<int, mFunc*> builtIns;

    mOperand *insertReg(Value *value);
    mOperand *handleValue(Value *value);
    mOperand *handleAddr(Value *value);
    mOperand *materialize(mOperand *oper);

    void handleUnaryOp(Inst *inst, const OpCode &code);
    void handleBinOp(Inst *inst, const OpCode &code);
    void handleCmpOp(Inst *inst, const OpCode &code);

    void handleZext(Inst *inst);
    void handleTrunc(Inst *inst);

    void handleAlloca(Inst *inst);
    void handleLoad(Inst *inst);
    void handleStore(Inst *inst);
    void handleGEP(Inst *inst);
    void handleCall(Inst *inst);
    void handlePHI(Inst *inst);

    void handleUBr(Inst *inst);
    void handleCBr(Inst *inst);
    void handleRet(Inst *inst);

    VirtReg *newVirtReg(TypeKind *type);
    PhyReg *getPhyReg(const Reg &reg);

    void emit(const Code &code, std::initializer_list<mOperand*> opers);
    void emitBr(const Code &code, mOperand *oper, mBlock *block);
    void emitArgs(std::vector<Value*> args);
    void emitCall(const std::string &name, unsigned numArgs);
    void emitPhi(mOperand *inst, std::vector<std::pair<mOperand*, mBlock*>> pairs);
    void emitRet();

    mOperand *genOpInst(const Code &code, std::initializer_list<mOperand*> opers, TypeKind *type);
    mOperand *genAddr(Value *value);

    void dispatch(const std::string &name);
    mFunc *genSys(int num);

    mBlock *getEpilogue();
    void reset();

  public:
    LowerPass (Module *module);
    mModule *getModule();

    void lower();
        
    void lowerFunc(Func *func);
    void lowerBlock(BasicBlock *block);
    void lowerInst(Inst *inst);

    void print(std::ostream &os);
};

Code getCode(const OpCode &opc);
Code getCode(const OpCode &opc, TypeKind *type);

Code getLoadCode(mOperand *oper);
Code getStoreCode(mOperand *oper);

}

#endif

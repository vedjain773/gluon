#ifndef BBLOCK_H
#define BBLOCK_H

#include "IR/Inst.hpp"
#include <vector>
#include <memory>

class BasicBlock {
private:
    Func *parent;                                      
    std::vector<std::unique_ptr<Instruction>> instructions;
    std::vector<BasicBlock*> predecessors;              
    std::vector<BasicBlock*> successors;

public:
    Func *getParent() const;

    void appendInstruction(std::unique_ptr<Instruction> inst);
    void insertInstructionBefore(Instruction *before, std::unique_ptr<Instruction> inst);
    void removeInstruction(Instruction *inst);

    auto begin(); auto end();
    Instruction *getFirstInst();
    Instruction *getLastInst();

    Instruction *getTerminator();
    bool hasTerminator();

    const std::vector<BasicBlock*> &getPredecessors();
    const std::vector<BasicBlock*> &getSuccessors();
    void addPredecessor(BasicBlock *bb);

    void print(std::ostream &os);
};

#endif

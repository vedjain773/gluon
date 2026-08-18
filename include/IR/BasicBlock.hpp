#ifndef BBLOCK_H
#define BBLOCK_H

#include "IR/Inst.hpp"
#include <vector>
#include <memory>

class Func;

class BasicBlock {
  private:
    std::string name;
    Func *parent;                                      
    std::vector<std::unique_ptr<Inst>> instructions;
    std::vector<BasicBlock*> predecessors;              
    std::vector<BasicBlock*> successors;

  public:
    BasicBlock(const std::string &name, Func *parent);
    static BasicBlock *Create(const std::string &name, Func *parent);

    Func *getParent();
    std::string getName();

    void appendInst(std::unique_ptr<Inst> inst);
    void insertInst(Inst *before, std::unique_ptr<Inst> inst);
    void removeInst(Inst *inst);

    Inst *getFirstInst();
    Inst *getLastInst();

    Inst *getTerminator();
    bool hasTerminator();

    std::vector<BasicBlock*> &getPredecessors();
    std::vector<BasicBlock*> &getSuccessors();
    void addPredecessor(BasicBlock *bb);

    void print(std::ostream &os);
};

#endif

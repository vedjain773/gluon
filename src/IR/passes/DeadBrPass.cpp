#include "IR/passes/DeadBrPass.hpp"
#include "IR/IRBuilder.hpp"

#include <vector>

void DeadBrPass::run(Func *func) {
    IRBuilder builder(func->getParent());
    std::vector<BasicBlock*> deadBlocks;

    for (auto &block: func->getBlocks()) {
        Inst *inst = block->getTerminator();
        if (inst == nullptr) continue;

        CondBrInst *brinst = dynamic_cast<CondBrInst*>(inst);
        if (brinst == nullptr) continue;

        Value *cond = brinst->getCond();
        ConstantInt *condConst = dynamic_cast<ConstantInt*>(cond);
        if (condConst == nullptr) continue;

        builder.setInsertPoint(block.get());
        uint64_t value = condConst->getValue() != 0;

        BasicBlock *live = value ? brinst->getThenBlock() : brinst->getElseBlock();
        BasicBlock *dead = value ? brinst->getElseBlock() : brinst->getThenBlock();
    
        builder.createBr(live);
        inst->eraseFromParent();

        for (auto &succDead: dead->getSuccessors())
            succDead->remPredecessor(dead);

        dead->remPredecessor(block.get());
    }

    for (auto &block: func->getBlocks()) {
        if (block->hasNPreds(0) && func->getEntryBlock() != block.get())
            deadBlocks.push_back(block.get()); 
    }

    for (auto &block: deadBlocks)
        func->removeBlock(block);
}

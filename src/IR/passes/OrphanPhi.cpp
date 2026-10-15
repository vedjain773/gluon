#include "IR/passes/OrphanPhi.hpp"

void OrphanPhi::run(Func *func) {
    for (auto &block: func->getBlocks()) {
        std::vector<PHINode*> &phis = block->phis();

        for (auto it = phis.begin(); it != phis.end();) {
            PHINode *phi = *it++;
            unsigned edges = phi->getNumEdges();
            
            if (edges == 1) {
                auto &[value, _] = phi->getPair(0);

                phi->replaceAllUsesWith(value);
                phi->eraseFromParent();
            }
        }
    }
}

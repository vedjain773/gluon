#include "IR/passes/ConstantProp.hpp"

#include <vector>

ConstantInt *ConstantProp::fold(OpCode opcode, ConstantInt *c1, ConstantInt *c2) {
    uint64_t v1 = c1->getValue();
    uint64_t v2 = c2->getValue();

    TypeKind *type = c1->getType();
    uint64_t res;

    switch (opcode) {
        case OpCode::ADD: res = v1 + v2;
        break;

        case OpCode::SUB: res = v1 - v2;
        break;
        
        case OpCode::MUL: res = v1 * v2;
        break;
        
        case OpCode::DIV: res = v1 / v2;
        break;
        
        case OpCode::REM: res = v1 % v2;
        break;
        
        case OpCode::GT: res = v1 > v2;
        break;

        case OpCode::GTE: res = v1 >= v2;
        break;

        case OpCode::LT: res = v1 < v2;
        break;

        case OpCode::LTE: res = v1 <= v2;
        break;

        case OpCode::EQ: res = v1 == v2;
        break;

        case OpCode::NEQ: res = v1 != v2;
        break;

        case OpCode::LAND: res = v1 && v2;
        break;

        case OpCode::LOR: res = v1 || v2;
        break;

        default: return nullptr;
    }

    return ConstantInt::Create(type, res);
}

void ConstantProp::run(Func *func) {
    std::vector<Inst*> erasables;

    for (auto &block: func->getBlocks()) {
        auto &insts = block->getInsts();

        for (auto it = insts.begin(); it != insts.end(); ++it) {
            Inst *inst = (*it).get();

            if (!inst->isBinary()) continue;

            ConstantInt *c1 = dynamic_cast<ConstantInt*>(inst->getOperand(0));
            ConstantInt *c2 = dynamic_cast<ConstantInt*>(inst->getOperand(1));

            if (c1 == nullptr || c2 == nullptr) return;

            ConstantInt *folded = fold(inst->getOpCode(), c1, c2);
            if (folded == nullptr) continue;

            inst->replaceAllUsesWith(folded);
            erasables.push_back(inst);
        }
    }

    for (auto &inst: erasables) inst->eraseFromParent();
}

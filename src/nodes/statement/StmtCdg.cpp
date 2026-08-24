#include "nodes/Statement.hpp"

void ExprStmt::codegen(CodegenVis &cdgvis) {
    expression->codegen(cdgvis);
}

void BlockStmt::codegen(CodegenVis &cdgvis) {
    for (size_t i = 0; i < statements.size(); i++) {
        statements[i]->codegen(cdgvis);

        if (statements[i]->isTerminator())
            break;
    }
}

void IfStmt::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    Value *cond = condition->codegen(cdgvis);

    if (!cond) {
        return;
    }

    Value *zero = ConstantInt::Create(getType("int"), 0);
    cond = Bldr->createCmp(OpCode::NEQ, cond, zero, "ifcond");

    Func *func = Bldr->getInsertBlock()->getParent();

    BasicBlock *thenRaw = BasicBlock::Create("if.then", func);

    auto mergeBB = BasicBlock::CreateDetached("if.after");
    auto elseBB = BasicBlock::CreateDetached("if.else");
 
    BasicBlock *mergeRaw = mergeBB.get();
    BasicBlock *elseRaw = elseBB.get();

    if (elseStmt != nullptr) {
        Bldr->createCondBr(cond, thenRaw, elseRaw);
    } else {
        Bldr->createCondBr(cond, thenRaw, mergeRaw);
    }

    Bldr->setInsertPoint(thenRaw);

    body->codegen(cdgvis);

    if (Bldr->getInsertBlock()->getTerminator() == nullptr) {
        Bldr->createBr(mergeRaw);
    }

    if (elseStmt != nullptr) {
        func->insertAtEnd(std::move(elseBB));
        Bldr->setInsertPoint(elseRaw);

        elseStmt->codegen(cdgvis);

        if (Bldr->getInsertBlock()->getTerminator() == nullptr) {
            Bldr->createBr(mergeRaw);
        }
    }

    func->insertAtEnd(std::move(mergeBB));
    Bldr->setInsertPoint(mergeRaw);
}

void ElseStmt::codegen(CodegenVis &cdgvis) {
    body->codegen(cdgvis);
}

void WhileStmt::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();

    Func *func = Bldr->getInsertBlock()->getParent();

    BasicBlock *condRaw = BasicBlock::Create("while.cond", func);
    auto bodyBB = std::make_unique<BasicBlock>("while.body", nullptr);
    auto afterBB = std::make_unique<BasicBlock>("while.after", nullptr);

    BasicBlock *bodyRaw = bodyBB.get();
    BasicBlock *afterRaw = afterBB.get();

    Bldr->createBr(condRaw);

    Bldr->setInsertPoint(condRaw);

    Value *cond = condition->codegen(cdgvis);

    if (!cond) {
        return;
    }

    Value *zero = ConstantInt::Create(getType("int"), 0);
    cond = Bldr->createCmp(OpCode::NEQ, cond, zero, "while.cond");

    Bldr->createCondBr(cond, bodyRaw, afterRaw);

    func->insertAtEnd(std::move(bodyBB));
    Bldr->setInsertPoint(bodyRaw);

    body->codegen(cdgvis);

    if (Bldr->getInsertBlock()->getTerminator() == nullptr) {
        Bldr->createBr(condRaw);
    }

    func->insertAtEnd(std::move(afterBB));
    Bldr->setInsertPoint(afterRaw);
}

void BreakStmt::codegen(CodegenVis &cdgvis) {
    //TBD 
}

void ContinueStmt::codegen(CodegenVis &cdgvis) {
    //TBD 
}

void ReturnStmt::codegen(CodegenVis &cdgvis) {
    Value *retVal = retExpr->codegen(cdgvis);
    cdgvis.builder->createRet(retVal);
}

void DeclStmt::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    Func *func = Bldr->getInsertBlock()->getParent();

    AllocaInst *alloca = Bldr->createEntryAlloca(type, name);

    if (expression != nullptr) {
        Value *initVal = expression->codegen(cdgvis);

        Bldr->createStore(initVal, alloca);
    }

    cdgvis.insertName(name, alloca);
}

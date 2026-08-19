#include "nodes/Function.hpp"

Func *Prototype::codegen(CodegenVis &cdgvis) {
    Module *module = (cdgvis.module).get();
    std::vector<Arg*> argVec;

    Func *func = Func::Create(funcName, module, retType, std::vector<Arg*> {});

    for (size_t i = 0; i < paramList.size(); i++) {
        Parameter *param = (paramList[i]).get();
        argVec.push_back(Arg::Create(param->type, param->name, func, i));
    }

    func->setArgs(argVec);

    return func;
}

void FuncDef::codegen(CodegenVis &cdgvis) {
    Module *module = (cdgvis.module).get();
    IRBuilder *Bldr = (cdgvis.builder).get();

    Func *func = module->getFunc(prototype->funcName);

    if (!func)
        func = prototype->codegen(cdgvis);

    BasicBlock *BB = BasicBlock::Create("entry", func);
    Bldr->setInsertPoint(BB);

    size_t i = 0;
    Prototype *proto = prototype.get();
    for (auto &arg : func->getArgs()) {
        Parameter *param = (proto->paramList[i++]).get();

        std::string paramAllocaStr = std::format("{}.addr", arg->getName()); 

        AllocaInst *alloca = Bldr->createEntryAlloca(param->type, paramAllocaStr);
        Bldr->createStore(arg, alloca);

        cdgvis.insertName(arg->getName(), alloca);
    }

    funcBody->codegen(cdgvis); 
}

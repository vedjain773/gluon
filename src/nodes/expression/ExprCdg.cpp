#include "nodes/Expression.hpp"

Value *IntExpr::codegen(CodegenVis &cdgvis) {
    return ConstantInt::Create(infType, Val);
}

Value *CharExpr::codegen(CodegenVis &cdgvis) {
    return ConstantInt::Create(infType, character);
}

Value *VarExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    AllocaInst *alloca = cdgvis.lookup(Name);

    if (!alloca) {
        return nullptr; 
    }

    return Bldr->createLoad(alloca->getValType(), alloca, Name);
}

Value *VarExpr::emitPtr(CodegenVis &cdgvis) {
    AllocaInst *alloca = cdgvis.lookup(Name);

    if (!alloca) {
        return nullptr; 
    }

    return alloca;
}

Value *DerefExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();

    Value *ptr = expr->emitPtr(cdgvis);

    if (isPointerType(expr->infType))
        ptr = expr->codegen(cdgvis);

    TypeKind *type = expr->infType->to;

    return Bldr->createLoad(type, ptr, "deref");
}

Value *DerefExpr::emitPtr(CodegenVis &cdgvis) {
    if (isArrayType(expr->infType))
        return expr->emitPtr(cdgvis);

    return expr->codegen(cdgvis);
}

Value *AddressExpr::codegen(CodegenVis &cdgvis) {
    return expr->emitPtr(cdgvis);
}

Value *SizeOfExpr::codegen(CodegenVis &cdgvis) {
    return ConstantInt::Create(getType("int"), argType->size);
}

Value *CastExpr::codegen(CodegenVis &cdgvis) {
    //TBD 
}

Value *UnaryExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    Value *val = Operand->codegen(cdgvis);

    switch (Op) {
        case Operators::MINUS: {
            return Bldr->createNeg(val, "neg");
        } break;

        case Operators::BANG: {
            return Bldr->createNot(val, "not"); 
        } break;

        default: {
            return val;
        }
    }
}

Value *BinaryExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    Value *left = LHS->codegen(cdgvis);
    Value *right = RHS->codegen(cdgvis);

    if (!left || !right)
        return nullptr;

    if (isPointerType(LHS->infType)) 
        return cdgvis.handlePtrArith({left, right, Op, LHS->infType->to});

    return cdgvis.handleBinOp({left, right, Op, infType});
}

Value *BinaryExpr::emitPtr(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();

    Value *left = LHS->emitPtr(cdgvis);
    Value *right = RHS->codegen(cdgvis);

    if (isArrayType(LHS->infType))
        return Bldr->createGEP(LHS->infType, left, 
                {ConstantInt::Create(getType("int"), 0), right}, "arroff");

    return nullptr;
}

Value *AssignExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();

    Value *addr = LHS->emitPtr(cdgvis);
    Value *exprVal = RHS->codegen(cdgvis);
    
    Bldr->createStore(exprVal, addr);
    return exprVal;
}

Value *EmptyExpr::codegen(CodegenVis &cdgvis) {
    return nullptr;
}

Value *CallExpr::codegen(CodegenVis &cdgvis) {
    IRBuilder *Bldr = (cdgvis.builder).get();
    Module *module = (cdgvis.module).get();
    Func *calleefunc = module->getFunc(callee);

    if (!calleefunc)
        return nullptr;

    if (calleefunc->getArgs().size() != args.size())
        return nullptr;

    std::vector<Value *> argVals;

    for (unsigned i = 0, e = args.size(); i != e; ++i) {
        argVals.push_back(args[i]->codegen(cdgvis));
        if (!argVals.back())
            return nullptr;
    }

    return Bldr->createCall(calleefunc, argVals, "calltmp");
}

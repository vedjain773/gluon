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

    return Bldr->createLoad(alloca->getType(), alloca, Name);
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

    switch (Op) {
        case Operators::PLUS: {
            return Bldr->createBinOp(OpCode::ADD, left, right, "add");
        } break;

        case Operators::MINUS: {
            return Bldr->createBinOp(OpCode::SUB, left, right, "sub");
        } break;

        case Operators::MULT: {
            return Bldr->createBinOp(OpCode::MUL, left, right, "mul");
        } break;

        case Operators::DIVIDE: {
            return Bldr->createBinOp(OpCode::DIV, left, right, "sdiv");
        } break;

        case Operators::MODULUS: {
            return Bldr->createBinOp(OpCode::REM, left, right, "srem");
        } break;

        case Operators::GREATER: {
            Value *gt = Bldr->createCmp(OpCode::GT, left, right, "compSGT");
            return Bldr->createZExt(gt, infType, "ext");
        } break;

        case Operators::GREATER_EQUALS: {
            Value *ge = Bldr->createCmp(OpCode::GTE, left, right, "compSGE");
            return Bldr->createZExt(ge, infType, "ext");
        } break;

        case Operators::LESS: {
            Value *lt = Bldr->createCmp(OpCode::LT, left, right, "compSLT");
            return Bldr->createZExt(lt, infType, "ext");
        } break;

        case Operators::LESS_EQUALS: {
            Value *le = Bldr->createCmp(OpCode::LTE, left, right, "compSLE");
            return Bldr->createZExt(le, infType, "ext");
        } break;

        case Operators::EQUALS: {
            Value *ee = Bldr->createCmp(OpCode::EQ, left, right, "compEE");
            return Bldr->createZExt(ee, infType, "ext");
        } break;

        case Operators::NOT_EQUALS: {
            Value *ne = Bldr->createCmp(OpCode::NEQ, left, right, "compNE");
            return Bldr->createZExt(ne, infType, "ext");
        } break;

        case Operators::AND: {
            Value *booland = Bldr->createBinOp(OpCode::LAND, left, right, "and");
            return Bldr->createZExt(booland, infType, "ext");
        } break;

        case Operators::OR: {
            Value *boolor = Bldr->createBinOp(OpCode::LOR, left, right, "or");
            return Bldr->createZExt(boolor, infType, "ext");
        } break;

        default:
            return left;
    }
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

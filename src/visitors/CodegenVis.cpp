#include "visitors/CodegenVis.hpp"
#include "nodes/Expression.hpp"

void CodegenVis::initModule(const std::string &fileName) {
    module = std::move(std::make_unique<Module>(fileName));
    builder = std::move(std::make_unique<IRBuilder>(module.get()) ); 
}

void CodegenVis::insertName(const std::string &name, AllocaInst *alloca) {
    table.insert({name, alloca});
}

Value *CodegenVis::handleBinOp(const OpConfig &opconfig) {
    auto &[left, right, op, type] = opconfig;
    IRBuilder *Bldr = builder.get(); 

    switch (op) {
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
            return Bldr->createCmp(OpCode::GT, left, right, "compSGT");
        } break;

        case Operators::GREATER_EQUALS: {
            return Bldr->createCmp(OpCode::GTE, left, right, "compSGE");
        } break;

        case Operators::LESS: {
            return Bldr->createCmp(OpCode::LT, left, right, "compSLT");
        } break;

        case Operators::LESS_EQUALS: {
            return Bldr->createCmp(OpCode::LTE, left, right, "compSLE");
        } break;

        case Operators::EQUALS: {
            return Bldr->createCmp(OpCode::EQ, left, right, "compEE");
        } break;

        case Operators::NOT_EQUALS: {
            return Bldr->createCmp(OpCode::NEQ, left, right, "compNE");
        } break;

        case Operators::AND: {
            return Bldr->createBinOp(OpCode::LAND, left, right, "and");
        } break;

        case Operators::OR: {
            return Bldr->createBinOp(OpCode::LOR, left, right, "or");
        } break;

        default:
            return left;
    }
}

Value *CodegenVis::handlePtrArith(const OpConfig &opconfig) {
    IRBuilder *Bldr = builder.get();
    auto &[left, right, op, type] = opconfig;

    Value *rightN = right;
    if (op == Operators::MINUS) {
        rightN = Bldr->createNeg(right, "neg");
    }

    return Bldr->createGEP(type, left, {rightN}, "ptroff");
}

void CodegenVis::clearTable() {
    table.clear();
}

AllocaInst *CodegenVis::lookup(std::string name) {
    return table[name];
}

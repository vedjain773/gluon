#ifndef EXPRESSION_H
#define EXPRESSION_H

#include "frontend/scanner/Token.hpp"
#include "utils/Scope.hpp"
#include "visitors/Visitor.hpp"
#include "visitors/CodegenVis.hpp"

#include <memory>
#include <string>

enum class Operators {
    // Unary
    BANG,
    MINUS,

    // Binary
    MODULUS,
    DIVIDE,
    MULT,
    PLUS,

    // Comparison
    GREATER,
    GREATER_EQUALS,
    LESS,
    LESS_EQUALS,

    // Equality
    EQUALS,
    NOT_EQUALS,

    // Logical
    AND,
    OR,

    // Assignment
    ASSIGN,
    SUM_ASSIGN,
    DIFF_ASSIGN,
    PROD_ASSIGN,
    QUOT_ASSIGN,
    MOD_ASSIGN
};

std::string getOpStr(Operators op);
Operators getOp(std::string op_str);

class Expression {
  public:
    TypeKind *infType;
    int line, column;

    Expression() = default;
    Expression(int tline, int tcol);

    virtual void accept(Visitor &visitor) = 0;
    virtual ~Expression() = default;
    
    virtual Value *codegen(CodegenVis &cdgvis) = 0; 
    virtual Value *emitPtr(CodegenVis &cdgvis) { return nullptr; }
    virtual bool isLValue() { return false; }
};

class IntExpr : public Expression {
  public:
    int Val;

    IntExpr(int value, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis); 
};

class CharExpr : public Expression {
  public:
    char character;

    CharExpr(char charac, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis); 
};

class VarExpr : public Expression {
  public:
    std::string Name;

    VarExpr(std::string name, int tline, int tcol);
    void accept(Visitor &visitor);
    bool isLValue();

    Value *codegen(CodegenVis &cdgvis);
    Value *emitPtr(CodegenVis &cdgvis);
};

class DerefExpr : public Expression {
  public:
    std::unique_ptr<Expression> expr;

    DerefExpr(std::unique_ptr<Expression> expression, int tline, int tcol);
    void accept(Visitor &visitor);
    bool isLValue();

    Value *codegen(CodegenVis &cdgvis);
    Value *emitPtr(CodegenVis &cdgvis);
};

class AddressExpr : public Expression {
  public:
    std::unique_ptr<Expression> expr;

    AddressExpr(std::unique_ptr<Expression> expression, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis); 
};

class SizeOfExpr : public Expression {
  public:
    TypeKind *argType;
    std::unique_ptr<Expression> expr;

    SizeOfExpr(std::unique_ptr<Expression> expression, int tline, int tcol);
    SizeOfExpr(TypeKind *typek, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis); 
};

class CastExpr : public Expression {
  public:
    TypeKind *from;
    TypeKind *to;
    std::unique_ptr<Expression> expr;

    CastExpr(std::unique_ptr<Expression> expression, TypeKind *from_tk,
             TypeKind *to_tk);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis); 
};

class UnaryExpr : public Expression {
  public:
    Operators Op;
    std::unique_ptr<Expression> Operand;

    UnaryExpr(Operators op, std::unique_ptr<Expression> operand, int tline,
              int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis);
};

class BinaryExpr : public Expression {
  public:
    Operators Op;
    std::unique_ptr<Expression> LHS;
    std::unique_ptr<Expression> RHS;

    BinaryExpr(Operators op, std::unique_ptr<Expression> lhs,
               std::unique_ptr<Expression> rhs, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis &cdgvis);
};

class AssignExpr : public Expression {
  public:
    std::unique_ptr<Expression> LHS;
    std::unique_ptr<Expression> RHS;

    AssignExpr(std::unique_ptr<Expression> lhs, std::unique_ptr<Expression> rhs,
               int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis& cdgvis);
};

class CompAssignExpr : public Expression {
  public:
    Operators Op;
    std::unique_ptr<Expression> LHS;
    std::unique_ptr<Expression> RHS;

    CompAssignExpr(Operators assignOp, std::unique_ptr<Expression> lhs,
                   std::unique_ptr<Expression> rhs, int tline, int tcol);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis& cdgvis);
};

class EmptyExpr : public Expression {
  public:
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis& cdgvis);
};

class CallExpr : public Expression {
  public:
    std::string callee;
    std::vector<std::unique_ptr<Expression>> args;

    CallExpr(std::string callee_name, int tline, int tcol);
    void add(std::unique_ptr<Expression> arg);
    void accept(Visitor &visitor);

    Value *codegen(CodegenVis& cdgvis);
};

class MemberAccessExpr : public Expression {
  public:
    std::unique_ptr<Expression> base;
    std::string fName;

    MemberAccessExpr(std::unique_ptr<Expression> baseExpr,
                     std::string fieldName, int line, int col);
    void accept(Visitor &visitor);
    bool isLValue();

    Value *codegen(CodegenVis& cdgvis);
};

#endif

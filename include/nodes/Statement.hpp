#ifndef STATEMENT_H
#define STATEMENT_H

#include "frontend/scanner/Token.hpp"
#include "nodes/Expression.hpp"
#include "nodes/ExternalDecl.hpp"
#include "utils/Scope.hpp"
#include "visitors/Visitor.hpp"
#include "IR/CodegenVis.hpp"
#include <vector>

class Statement {
  public:
    int line, column;

    Statement() = default;
    Statement(int tline, int tcol);

    virtual ~Statement() = default;
    virtual void accept(Visitor &visitor) = 0;
    
    virtual void codegen(CodegenVis &cdgvis) = 0; 

    virtual bool isTerminator() {
        return false;
    }
};

class EmptyStmt : public Statement {
    void accept(Visitor &visitor);
};

class ExprStmt : public Statement {
  public:
    std::unique_ptr<Expression> expression;

    ExprStmt(std::unique_ptr<Expression> expr);
    void accept(Visitor &visitor);

    void codegen(CodegenVis &cdgvis); 
};

class BlockStmt : public Statement {
  public:
    std::vector<std::unique_ptr<Statement>> statements;

    void addStmt(std::unique_ptr<Statement> stmt);
    void accept(Visitor &visitor);
 
    void codegen(CodegenVis &cdgvis);  
};

class IfStmt : public Statement {
  public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;
    std::unique_ptr<Statement> elseStmt;

    IfStmt(std::unique_ptr<Expression> condition,
           std::unique_ptr<Statement> ifbody,
           std::unique_ptr<Statement> elsestmt);
    void accept(Visitor &visitor);

    void codegen(CodegenVis &cdgvis);
};

class ElseStmt : public Statement {
  public:
    std::unique_ptr<Statement> body;

    ElseStmt(std::unique_ptr<Statement> elsebody);
    void accept(Visitor &visitor);

    void codegen(CodegenVis &cdgvis);
};

class WhileStmt : public Statement {
  public:
    std::unique_ptr<Expression> condition;
    std::unique_ptr<Statement> body;

    WhileStmt(std::unique_ptr<Expression> condn,
              std::unique_ptr<Statement> whilebody);
    void accept(Visitor &visitor);

    void codegen(CodegenVis &cdgvis);
};

class BreakStmt : public Statement {
  public:
    BreakStmt(int tline, int tcol);
    void accept(Visitor &visitor);
    bool isTerminator();

    void codegen(CodegenVis &cdgvis);
};

class ContinueStmt : public Statement {
  public:
    ContinueStmt(int tline, int tcol);
    void accept(Visitor &visitor);
    bool isTerminator();

    void codegen(CodegenVis &cdgvis);
};

class ReturnStmt : public Statement {
  public:
    std::unique_ptr<Expression> retExpr;

    ReturnStmt(std::unique_ptr<Expression> retexpr);
    void accept(Visitor &visitor);
    bool isTerminator();

    void codegen(CodegenVis &cdgvis);
};

class DeclStmt : public Statement {
  public:
    TypeKind *type;
    std::string name;
    std::unique_ptr<Expression> expression;

    DeclStmt(TypeKind *tk, std::string varname,
             std::unique_ptr<Expression> expr, int tline, int tcol);
    void accept(Visitor &visitor);

    void codegen(CodegenVis &cdgvis);
};

#endif

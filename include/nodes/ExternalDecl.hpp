#ifndef EXTERNALDECL_H
#define EXTERNALDECL_H

#include "visitors/Visitor.hpp"
#include "visitors/CodegenVis.hpp"

class ExternalDecl {
  public:
    virtual void accept(Visitor &visitor) = 0;
    virtual void codegen(CodegenVis &cdgvis) = 0;
    virtual ~ExternalDecl() = default;
};

#endif

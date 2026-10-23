#ifndef SCOPE_H
#define SCOPE_H

#include "frontend/scanner/Token.hpp"

#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <optional>
#include <initializer_list>

using size_t = std::size_t;

enum class SymbolKind { VARIABLE, FUNCTION };

enum class BuiltIn { WRITE, EXIT };

enum class TypeEnum {
    // Primitives
    BUILTIN,

    // Aggregate
    POINTER,
    ARRAY,
    STRUCT,

    // Error
    ERROR
};

struct TypeKind {
    TypeEnum type;

    std::string name;
    size_t size;
    size_t align;
    bool isSigned = true;

    TypeKind *to = nullptr;
};

struct Symbol {
    SymbolKind kind;
    TypeKind *type;

    std::vector<TypeKind *> params;
};

std::optional<BuiltIn> getBuiltIn(const std::string &name);
std::string getBuiltInName(const BuiltIn &kind);

extern std::unordered_map<std::string, std::unique_ptr<TypeKind>> typeTable;
TypeKind *TokToType(TokenType tk);

TypeKind *getType(std::string typeName);
TypeKind *getPtrTo(std::string typeName);
TypeKind *getArrType(std::string typeName, int numOfElements);

bool isPointerType(TypeKind *typek);
bool isArrayType(TypeKind *typek);
bool isErrorType(TypeKind *typek);

int getNumElements(TypeKind *typek);

class Scope {
  public:
    std::map<std::string, Symbol> symTable;

    void addRow(std::string name, TypeKind *type, SymbolKind symKind);
    bool search(std::string name);

    void addParam(std::string name, TypeKind *type);
    void addParams(std::string name, std::initializer_list<TypeKind*> types);
    size_t getNumParams(std::string name);
    
    TypeKind *getSymType(std::string name);
    SymbolKind getSymKind(std::string name);
    std::vector<TypeKind *> getParams(std::string name);
};

#endif

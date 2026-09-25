#include "utils/Scope.hpp"
#include <iostream>
#include <format>

std::unordered_map<std::string, std::unique_ptr<TypeKind>> typeTable = [] {
   std::unordered_map<std::string, std::unique_ptr<TypeKind>> m;

    TypeKind intType = {
        .type = TypeEnum::BUILTIN,
        .name = "int",
        .size = 4,
        .align = 4,
        .to = nullptr
    }; 

    TypeKind uint8Type = {
        .type = TypeEnum::BUILTIN,
        .name = "uint8_t",
        .size = 1,
        .align = 1,
        .isSigned = false,
        .to = nullptr
    };

    TypeKind uint16Type = {
        .type = TypeEnum::BUILTIN,
        .name = "uint16_t",
        .size = 2,
        .align = 2,
        .isSigned = false,
        .to = nullptr
    };

    TypeKind charType = {
        .type = TypeEnum::BUILTIN,
        .name = "char",
        .size = 1,
        .align = 1,
        .to = nullptr
    };

    TypeKind voidType = {
        .type = TypeEnum::BUILTIN,
        .name = "void",
        .size = 0,
        .align = 0,
        .to = nullptr
    };

    TypeKind nullType = {
        .type = TypeEnum::BUILTIN,
        .name = "null",
        .size = 0,
        .align = 0,
        .to = nullptr
    };

    TypeKind errType = {
        .type = TypeEnum::ERROR,
        .name = "error",
        .size = 0,
        .align = 0,
        .to = nullptr
    };
        
    m.emplace("int", std::make_unique<TypeKind>(intType));
    m.emplace("uint8_t", std::make_unique<TypeKind>(uint8Type));
    m.emplace("uint16_t", std::make_unique<TypeKind>(uint16Type));
    m.emplace("char", std::make_unique<TypeKind>(charType));
    m.emplace("void", std::make_unique<TypeKind>(voidType));
    m.emplace("null", std::make_unique<TypeKind>(nullType));
    m.emplace("error", std::make_unique<TypeKind>(errType));

    return m;
}();

TypeKind *TokToType(TokenType tk) {
    switch (tk) {
        case TokenType::INT: {
            return typeTable["int"].get();
        } break;

        case TokenType::UINT8: {
            return typeTable["uint8_t"].get();
        } break;

        case TokenType::UINT16: {
            return typeTable["uint16_t"].get();
        } break;

        case TokenType::CHAR: {
            return typeTable["char"].get();
        } break;

        default:
            return typeTable["void"].get();
    }
}

TypeKind *getType(std::string typeName) {
    int size = typeName.size();

    if (typeTable.count(typeName) != 0) {
        return typeTable[typeName].get();
    } else if (typeName[size - 1] == '*') {
        TypeKind *base = typeTable[typeName.substr(0, size - 1)].get();
        
        TypeKind ptrType = {
            .type = TypeEnum::POINTER,
            .name = typeName,
            .size = 8,
            .align = 8,
            .to = base
        };

        std::unique_ptr<TypeKind> newType = std::make_unique<TypeKind>(ptrType);

        TypeKind *newType_raw = newType.get();

        typeTable[typeName] = std::move(newType);
        return newType_raw;
    }

    return typeTable["null"].get();
}

TypeKind *getPtrTo(std::string typeName) {
    std::string newTypeName = std::format("{}*", typeName);

    if (typeTable.count(newTypeName))
        return typeTable[newTypeName].get();

    TypeKind *base = typeTable[typeName].get();
   
    TypeKind ptrType = {
        .type = TypeEnum::POINTER,
        .name = newTypeName,
        .size = 8,
        .align = 8,
        .to = base
    };

    std::unique_ptr<TypeKind> newType = std::make_unique<TypeKind>(ptrType);

    TypeKind *newType_raw = newType.get();

    typeTable[newTypeName] = std::move(newType);
    return newType_raw;
}

TypeKind *getArrType(std::string typeName, int numOfElements) {
    TypeKind *base = getType(typeName);
    size_t baseSize = base->size;

    size_t arrSize = numOfElements * baseSize;

    std::string newTypeName = std::format("{}[{}]", typeName, numOfElements);

    if (typeTable.count(newTypeName)) return typeTable[newTypeName].get();
        
    TypeKind arrType = {
        .type = TypeEnum::ARRAY, 
        .name = newTypeName,
        .size = arrSize,
        .align = base->align,
        .to = base 
    };

    std::unique_ptr<TypeKind> newType = std::make_unique<TypeKind>(arrType);

    TypeKind *newType_raw = newType.get();

    typeTable[newTypeName] = std::move(newType);
    return newType_raw;
}

bool isPointerType(TypeKind *typek) {
    return typek->type == TypeEnum::POINTER;
}

bool isArrayType(TypeKind *typek) {
    return typek->type == TypeEnum::ARRAY;
}

bool isErrorType(TypeKind *typek) {
    return typek->type == TypeEnum::ERROR;
}

int getNumElements(TypeKind *typek) {
    int arrSize = typek->size;
    int elementSize = typek->to->size;

    return (arrSize / elementSize);
}

void Scope::addRow(std::string name, TokenType tokentype, SymbolKind symKind) {
    Symbol symbol;
    symbol.type = TokToType(tokentype);
    symbol.kind = symKind;

    symTable.insert({name, symbol});
}

void Scope::addRow(std::string name, TypeKind *type, SymbolKind symKind) {
    Symbol symbol;
    symbol.type = type;
    symbol.kind = symKind;

    symTable.insert({name, symbol});
}

bool Scope::search(std::string name) {
    return symTable.count(name);
}

void Scope::addParam(std::string name, TypeKind *type) {
    Symbol &sym = symTable[name];
    sym.params.push_back(type);
}

size_t Scope::getNumParams(std::string name) {
    Symbol &sym = symTable[name];
    return sym.params.size();
}

TypeKind *Scope::getSymType(std::string name) {
    Symbol &sym = symTable[name];
    return sym.type;
}

SymbolKind Scope::getSymKind(std::string name) {
    Symbol &sym = symTable[name];
    return sym.kind;
}

std::vector<TypeKind *> Scope::getParams(std::string name) {
    Symbol &sym = symTable[name];
    return sym.params;
}

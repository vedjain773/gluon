#ifndef MODULE_H
#define MODULE_H

#include "IR/Func.hpp"
#include <vector>
#include <memory>

class Module {
private:
    std::string name;
    std::vector<std::unique_ptr<Func>> functions;

public:
    Module(const std::string &name = "");
    Func *appendFunc(const std::string &name, TypeKind *returnType,
                      std::vector<TypeKind*> paramTypes);
    Func *appendFunc(std::unique_ptr<Func> func);

    Func *getFunc(const std::string &name);

    void print(std::ostream &os);
};

#endif

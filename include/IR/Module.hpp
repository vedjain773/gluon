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
    Func *createFunc(const std::string &name, TypeKind *returnType,
                      std::vector<TypeKind*> paramTypes);
    Func *getFunc(const std::string &name); 

    auto begin(); auto end();
    void print(std::ostream &os);
};

#endif

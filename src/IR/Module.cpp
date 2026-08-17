#include "IR/Module.hpp"

Module::Module(const std::string &name): name(name) {}

Func *Module::appendFunc(const std::string &name, 
        TypeKind *returnType, std::vector<TypeKind*> paramTypes) {

    auto func = std::make_unique<Func>(name, this, returnType, paramTypes);
    functions.push_back(std::move(func));
    return functions.back().get();
}

Func *Module::appendFunc(std::unique_ptr<Func> func) {
    functions.push_back(std::move(func));
    return functions.back().get();
}

Func *Module::getFunc(const std::string &name) {
    for (auto &func: functions) {
        if (func->getName() == name)
            return func.get();
    }

    return nullptr;
}

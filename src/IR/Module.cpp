#include "IR/Module.hpp"
#include <format>

Module::Module(const std::string &name): name(name) {}

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

void Module::print(std::ostream &os) {
    os << std::format(";{}\n\n", name);

    for (auto &func: functions) {
        func->print(os);
        os << '\n';
    }
} 

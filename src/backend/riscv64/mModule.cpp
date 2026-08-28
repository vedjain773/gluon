#include "backend/riscv64/mModule.hpp"

using namespace RISCV;

mModule::mModule(Module &mod): 
    name(mod.getName()) {}

mModule *mModule::Create(Module &mod) {
    return new mModule(mod);
}

mFunc *mModule::appendFunc(std::unique_ptr<mFunc> func) {
    functions.push_back(std::move(func));
    return functions.back().get();
}

void mModule::print(std::ostream &os) {
    os << std::format(";Module: {}\n\n", name);

    for (auto &func: functions) {
        func->print(os);
    }
}

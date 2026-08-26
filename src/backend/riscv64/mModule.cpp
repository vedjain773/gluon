#include "backend/riscv64/mModule.hpp"

using namespace RISCV;

mModule::mModule(Module &mod): 
    name(mod.getName()) {}

mFunc *mModule::appendFunc(std::unique_ptr<mFunc> func) {
    functions.push_back(std::move(func));
    return functions.back().get();
}

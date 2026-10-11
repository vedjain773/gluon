#include "IR/Opt.hpp"

Opt::Opt(Module *mod):
    mod(mod) {}

void Opt::addPass(Pass *pass) {
    passes.push_back(pass);
}

void Opt::optimize() {
    for (auto &func: mod->getFuncs()) {
        for (auto &pass: passes) pass->run(func.get());
    }
}

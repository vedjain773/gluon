#include "visitors/CodegenVis.hpp"

void CodegenVis::initModule(const std::string &fileName) {
    module = std::move(std::make_unique<Module>(fileName));
    builder = std::move(std::make_unique<IRBuilder>(module.get()) ); 
}

void CodegenVis::insertName(const std::string &name, AllocaInst *alloca) {
    table.insert({name, alloca});
}

AllocaInst *CodegenVis::lookup(std::string name) {
    return table[name];
}

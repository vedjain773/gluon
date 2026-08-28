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
    Func *appendFunc(std::unique_ptr<Func> func);

    std::string getName();
    Func *getFunc(const std::string &name);
   
    std::vector<std::unique_ptr<Func>> &getFuncs();

    void print(std::ostream &os);
};

#endif

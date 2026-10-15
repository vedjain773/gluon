target_sources(gluon PRIVATE
    BasicBlock.cpp
    Func.cpp
    Inst.cpp
    IRBuilder.cpp
    Module.cpp
    Value.cpp

    Opt.cpp
    passes/Mem2Reg.cpp
    passes/ConstantProp.cpp
    passes/DeadBrPass.cpp
    passes/OrphanPhi.cpp
)

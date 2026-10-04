target_sources(gluon PRIVATE
    mInst.cpp
    mBlock.cpp
    mFunc.cpp
    mModule.cpp

    riscv64/Emitter.cpp
    riscv64/LowerPass.cpp
    riscv64/mOperand.cpp
    riscv64/RegAlloc.cpp
    riscv64/FramePass.cpp
) 

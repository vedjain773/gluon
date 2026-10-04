#!/bin/bash

total_files=0
passed=0
for file in tests/*.c
do
    expected=$(grep "EXPECTED:" "$file" | cut -d':' -f2 | xargs)
    
    echo -ne "\r\033[K\e[34m[Testing]\e[0m: $file"

    total_files=$((total_files+1))
    for mode in normal 
    do
        
        ./gluon "$file" --print-asm -o out/prog.s 
        label="[STD]"

        riscv64-linux-gnu-as out/prog.s -o out/prog.o
        riscv64-linux-gnu-gcc -static out/prog.o -o out/prog

        if [ $? -ne 0 ]; then
            echo -e "\033[31m [FAIL] \033[0m $label $file linker error"
            continue
        fi

        ./out/prog
        actual=$?

        if [ "$actual" = "$expected" ]; then
          passed=$((passed+1))
        else
          echo -e "\033[31m [FAIL] \033[0m $label $file expected=$expected got=$actual"
        fi

    done
done

rm -f out/prog*

total_tests=$((total_files))
echo -ne "\r\033[KTotal tests passed: $passed / $total_tests\n"

cmake_minimum_required(VERSION 3.20)

project(gluon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

file(GLOB SRC_FILES
    src/*.cpp
    src/frontend/scanner/*.cpp
    src/frontend/parser/*.cpp
    src/nodes/*.cpp
    src/nodes/expression/*.cpp
    src/nodes/statement/*.cpp
    src/nodes/function/*.cpp
    src/visitors/*.cpp
    src/passes/*.cpp
    src/utils/*.cpp
)

add_executable(gluon ${SRC_FILES})

target_include_directories(gluon PRIVATE
    include
    tabulate/include
)

target_compile_options(gluon PRIVATE
    -Wall
)

set_target_properties(gluon PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

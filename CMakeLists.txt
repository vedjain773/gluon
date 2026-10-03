cmake_minimum_required(VERSION 3.20)

project(gluon LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_executable(gluon ${SRC_FILES})

target_sources(gluon PRIVATE src/main.cpp)

add_subdirectory(src/frontend)
add_subdirectory(src/IR)
add_subdirectory(src/backend)
add_subdirectory(src/nodes)
add_subdirectory(src/utils)
add_subdirectory(src/visitors)

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

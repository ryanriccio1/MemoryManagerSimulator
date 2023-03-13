cmake_minimum_required(VERSION 3.25.0)

project(MemoryManagerSimulator)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(-fcommon)
endif()

add_subdirectory(DataStructures)
add_subdirectory(MemoryManagerSimulator)
add_subdirectory(MemoryManagerSimulator.GUI)

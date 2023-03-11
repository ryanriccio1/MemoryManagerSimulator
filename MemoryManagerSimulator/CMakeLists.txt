set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.c)
list(REMOVE_ITEM SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/driver.c)


add_library(MemoryManagerSimulator STATIC ${SRC_FILES})
target_include_directories(MemoryManagerSimulator BEFORE PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
target_include_directories(MemoryManagerSimulator BEFORE PUBLIC ../DataStructures/include/)
target_link_libraries(MemoryManagerSimulator DataStructures)
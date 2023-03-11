set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


find_package(OpenGL REQUIRED)

add_subdirectory(glfw)
add_subdirectory(fmt)
include_directories(glfw/include)
include_directories(${OPENGL_INCLUDE_DIR})


file(GLOB SRC_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB IM_GUI_SRC ${CMAKE_CURRENT_LIST_DIR}/imgui/*.cpp)
set(BACKENDS 
${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_glfw.cpp
${CMAKE_CURRENT_LIST_DIR}/imgui/backends/imgui_impl_opengl3.cpp
)

add_executable(MemoryManagerSimulatorGui WIN32 ${SRC_FILES} ${IM_GUI_SRC} ${BACKENDS})
target_link_libraries(MemoryManagerSimulatorGui ${OPENGL_LIBRARIES} glfw  ${GLFW_LIBRARIES})
target_link_libraries(MemoryManagerSimulatorGui fmt::fmt)
target_link_libraries(MemoryManagerSimulatorGui DataStructures)
target_link_libraries(MemoryManagerSimulatorGui MemoryManagerSimulator)

target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ../MemoryManagerSimulator/)
target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ../DataStructures/include/)
target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ${CMAKE_CURRENT_LIST_DIR}/)
target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui/)
target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ${CMAKE_CURRENT_LIST_DIR}/imgui/backends/)
target_include_directories(MemoryManagerSimulatorGui BEFORE PUBLIC ${OPENGL_INCLUDE_DIR})


add_custom_command(TARGET MemoryManagerSimulatorGui 
                   POST_BUILD
                   COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:MemoryManagerSimulatorGui> ../../)
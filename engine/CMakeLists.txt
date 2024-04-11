file(GLOB SRCS *.cpp)
add_library(engine STATIC ${SRCS})
target_link_libraries(engine imgui glfw Vulkan::Vulkan)
target_include_directories(engine PRIVATE ${Vulkan_INCLUDE_DIR}/vma)

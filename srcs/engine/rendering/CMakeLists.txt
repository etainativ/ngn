file(GLOB SRCS *.cpp)
add_library(rendering STATIC ${SRCS})
target_link_libraries(rendering imgui glfw Vulkan::Vulkan)
target_include_directories(rendering PRIVATE ${Vulkan_INCLUDE_DIR}/vma ${Vulkan_INCLUDE_DIR})

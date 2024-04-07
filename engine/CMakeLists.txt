find_package(Vulkan REQUIRED FATAL_ERROR)
find_package(glfw3 REQUIRED FATAL_ERROR)
find_library(STB stb)

file(GLOB SRCS *.cpp)
add_library(vulkan_bootstrap STATIC VkBootstrap.cpp)
add_library(engine STATIC ${SRCS})
target_link_libraries(engine vulkan_bootstrap imgui stb glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)

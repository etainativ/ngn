file(GLOB SRCS imgui/*.cpp 
	imgui/backends/imgui_impl_vulkan.cpp 
	imgui/backends/imgui_impl_glfw.cpp)
add_library(imgui STATIC ${SRCS})
target_include_directories(imgui PRIVATE ${Vulkan_INCLUDE_DIR})

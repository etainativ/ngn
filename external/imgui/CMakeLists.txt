file(GLOB SRCS *.cpp backends/*.cpp)
add_library(imgui STATIC ${SRCS})
target_include_directories(imgui PRIVATE ${Vulkan_INCLUDE_DIR})

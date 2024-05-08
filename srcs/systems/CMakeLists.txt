file(GLOB SRCS *.cpp)
add_library(systems STATIC ${SRCS})
target_include_directories(systems PRIVATE ${Vulkan_INCLUDE_DIR}/vma ${Vulkan_INCLUDE_DIR})

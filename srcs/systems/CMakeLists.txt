file(GLOB SRCS *.cpp)
add_library(systems STATIC systems.cpp)
target_sources(systems INTERFACE ${SRCS})
target_include_directories(systems PRIVATE ${Vulkan_INCLUDE_DIR}/vma ${Vulkan_INCLUDE_DIR})

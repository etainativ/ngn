file(GLOB SRCS *.cpp)

add_library(engine STATIC ${SRCS})
target_link_libraries(engine common_systems)
target_include_directories(engine PRIVATE ${Vulkan_INCLUDE_DIR}/vma ${Vulkan_INCLUDE_DIR})

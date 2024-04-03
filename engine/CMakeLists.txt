find_package(Vulkan REQUIRED FATAL_ERROR) # error
find_package(glfw3 REQUIRED FATAL_ERROR) # error

add_library(vulkan_bootstrap STATIC VkBootstrap.cpp)
add_library(engine STATIC engine.cpp)
target_link_libraries(engine vulkan_bootstrap glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
target_include_directories(vulkan_bootstrap PUBLIC "${PROJECT_SOURCE_DIR}/include")
target_include_directories(engine PUBLIC "${PROJECT_SOURCE_DIR}/include")

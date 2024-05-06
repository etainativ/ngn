add_executable(${PROJECT_NAME} main.cpp)
add_subdirectory(engine)
add_subdirectory(systems)

target_link_libraries(${PROJECT_NAME} systems engine)
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR}/vma)

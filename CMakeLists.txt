if (WIN32)
	SET(CMAKE_SYSTEM_NAME Windows)
	SET(CMAKE_CXX_FLAGS "--static")
	SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc-posix)
	SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++-posix)
endif()

cmake_minimum_required(VERSION 3.10)
project(swe)

find_package(Vulkan REQUIRED FATAL_ERROR)

include_directories(include)
include_directories(external/imgui)
include_directories(external/tinygltf)
include_directories(external/entt/src)
include_directories(external/glfw/include)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")

add_subdirectory(srcs/engine)
add_subdirectory(srcs/systems)
add_subdirectory(srcs/utils)
add_subdirectory(external/imgui)
add_subdirectory(external/glfw)

add_executable(${PROJECT_NAME} srcs/main.cpp)
target_link_libraries(${PROJECT_NAME} systems engine)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND "${CMAKE_COMMAND}" -E copy_directory ../models models
	COMMAND "${CMAKE_COMMAND}" -E copy_directory ../shaders shader
)

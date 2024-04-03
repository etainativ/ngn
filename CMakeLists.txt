cmake_minimum_required(VERSION 3.10)
project(swe)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(BUILD_MODE DEBUG)
SET(CMAKE_CXX_FLAGS "-Wall -g -ggdb3")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(${PROJECT_NAME} main.cpp)
add_subdirectory(engine)
target_link_libraries(${PROJECT_NAME} engine)
target_include_directories(
	${PROJECT_NAME} PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/include")

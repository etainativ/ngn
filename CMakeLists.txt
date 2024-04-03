cmake_minimum_required(VERSION 3.10)
project(swe)

SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_BUILD_TYPE DEBUG)
SET(CMAKE_CXX_FLAGS_DEBUG "-Wall -g -ggdb3")
SET(CMAKE_CXX_FLAGS_RELEASE "-Wall -O3")

add_executable(${PROJECT_NAME} main.cpp)
add_subdirectory(engine)
target_link_libraries(${PROJECT_NAME} engine)
target_include_directories(
	${PROJECT_NAME} PUBLIC
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/include")

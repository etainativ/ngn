
file(GLOB SRCS *.cpp backends/*.cpp)
add_library(imgui STATIC ${SRCS})

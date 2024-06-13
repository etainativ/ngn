file(GLOB SRCS *.cpp)

add_library(server_funcs STATIC ${SRCS})
target_link_libraries(server_funcs networking)

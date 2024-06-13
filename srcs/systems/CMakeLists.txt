file(GLOB CLIENT_SRCS client/*.cpp)
file(GLOB SERVER_SRCS server/*.cpp)
file(GLOB COMMON_SRCS common/*.cpp)

add_library(client_systems STATIC ${CLIENT_SRCS})
add_library(server_systems STATIC ${SERVER_SRCS})
add_library(common_systems STATIC ${COMMON_SRCS})

target_include_directories(client_systems PRIVATE ${Vulkan_INCLUDE_DIR}/vma ${Vulkan_INCLUDE_DIR})
target_link_libraries(server_systems PRIVATE server_funcs)

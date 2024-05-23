mkdir winbuild; cd winbuild
VULKAN_SDK=/mnt/c/VulkanSDK/1.3.280.0 cmake -DWIN32=1 -DProtobuf_LIBRARIES="/mnt/c/Program\ Files/protobuf/lib" ..

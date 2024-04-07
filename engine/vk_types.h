#pragma once
#include <vulkan/vulkan_core.h>
#include "vk_mem_alloc.h"
#include <glm/glm.hpp>


struct GPUDrawPushConstants {
    glm::mat4 worldMatrix;
    VkDeviceAddress vertexBuffer;
};


struct AllocatedBuffer {
    VkBuffer buffer;
    VmaAllocation allocation;
    VmaAllocationInfo info;
    VkDeviceAddress address;
};

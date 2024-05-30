
#include <vulkan/vulkan_core.h>
namespace vk {
    namespace init {
	inline VkCommandBufferBeginInfo cmdBeginInfo() {
	    VkCommandBufferBeginInfo cmdBeginInfo = {};
	    cmdBeginInfo.sType             = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	    cmdBeginInfo.pNext             = nullptr;
	    cmdBeginInfo.pInheritanceInfo  = nullptr;
	    cmdBeginInfo.flags             = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	    return cmdBeginInfo;
	}

	inline VkImageCreateInfo imageCreateInfo() {
	    VkImageCreateInfo imageCI  = {};
	    imageCI.sType              = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	    imageCI.pNext              = nullptr;
	    return imageCI;
	}

	inline VkImageViewCreateInfo imageViewCreateInfo() {
	    VkImageViewCreateInfo imageViewCI = {};
	    imageViewCI.sType                 = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	    imageViewCI.pNext                 = nullptr;
	    return imageViewCI;
	}

	inline VkRect2D createRect2D(VkExtent3D imageExtent) {

	    VkRect2D scissor      = {};
	    scissor.offset.x      = 0;
	    scissor.offset.y      = 0;
	    scissor.extent.width  = imageExtent.width;
	    scissor.extent.height = imageExtent.height;
	    return scissor;
	}

	inline VkImageSubresourceRange imageSubresourceRange(
		VkImageAspectFlags aspectMask) {

	    VkImageSubresourceRange imageSubresourceRange = {};
	    imageSubresourceRange.aspectMask              = aspectMask;
	    imageSubresourceRange.baseMipLevel            = 0;
	    imageSubresourceRange.levelCount              = VK_REMAINING_MIP_LEVELS;
	    imageSubresourceRange.baseArrayLayer          = 0;
	    imageSubresourceRange.layerCount              = VK_REMAINING_ARRAY_LAYERS;
	    return imageSubresourceRange;
	}

	inline VkDependencyInfo createDependencuInfo(
		VkImage               image,
		VkImageLayout         currentLayout,
		VkImageLayout         newLayout,
		VkPipelineStageFlags2 srcStageMask,
		VkAccessFlags2        srcAccessMask,
		VkPipelineStageFlags2 dstStageMask,
		VkAccessFlags2        dstAccessMask,
		VkImageSubresourceRange subresourceRange) {

	    VkImageMemoryBarrier2 imageBarrier = {};
	    imageBarrier.sType                 = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2;
	    imageBarrier.pNext                 = nullptr;
	    imageBarrier.srcStageMask          = srcStageMask;
	    imageBarrier.srcAccessMask         = srcAccessMask;
	    imageBarrier.dstStageMask          = dstStageMask;
	    imageBarrier.dstAccessMask         = dstAccessMask;
	    imageBarrier.oldLayout             = currentLayout;
	    imageBarrier.newLayout             = newLayout;
	    imageBarrier.subresourceRange      = subresourceRange;
	    imageBarrier.image                 = image;

	    VkDependencyInfo depInfo        = {};
	    depInfo.sType                   = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
	    depInfo.pNext                   = nullptr;
	    depInfo.imageMemoryBarrierCount = 1;
	    depInfo.pImageMemoryBarriers    = &imageBarrier;
	    return depInfo;

	}
        inline VkCommandPoolCreateInfo commandPoolCreateInfo(
		uint32_t queueFamilyIndex) {

	    VkCommandPoolCreateInfo commandPoolCI = {};
	    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	    commandPoolCI.queueFamilyIndex = queueFamilyIndex;
	    commandPoolCI.pNext = nullptr;
	    return commandPoolCI;
	}

    }
}

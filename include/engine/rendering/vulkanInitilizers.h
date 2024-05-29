
#include <vulkan/vulkan_core.h>
namespace vk {
    namespace init {
	inline VkCommandBufferBeginInfo cmdBeginInfo() {
	    VkCommandBufferBeginInfo cmdBeginInfo {};
	    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	    cmdBeginInfo.pNext = nullptr;
	    cmdBeginInfo.pInheritanceInfo = nullptr;
	    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	    return cmdBeginInfo;
	}

	inline VkImageCreateInfo imageCreateInfo() {
	    VkImageCreateInfo imageCI = {};
	    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	    imageCI.pNext = nullptr;
	    return imageCI;
	}

	inline VkImageViewCreateInfo imageViewCreateInfo() {
	    VkImageViewCreateInfo imageViewCI = {};
	    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	    imageViewCI.pNext = nullptr;
	    return imageViewCI;
	}
    }
}

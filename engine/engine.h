#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"
#include <vector>


constexpr int FRAME_OVERLAP = 2;


struct FrameData {
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;
    VkSemaphore swapchainSemaphore, renderSemaphore;
    VkFence renderFence;
};


class Engine {
    public:
	Engine();
	~Engine();

	VkDevice device;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;

	VkAllocationCallbacks *pAllocator= nullptr;
	VmaAllocator allocator;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	VkExtent2D swapchainExtent;


	int width = 640;
	int height = 800;
	bool bQuit = false;
	void run();

    private:
	int frameNumber = 0;

	GLFWwindow *window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice phyDevice = VK_NULL_HANDLE;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	FrameData frames[FRAME_OVERLAP];

	void initWindow();
	void createSurface();
	void initVulkan();
	void initSwapchain();
	void initFrameData();
	void draw();

};

#pragma once

#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"
#include <vector>
#include <deque>
#include <functional>


constexpr int FRAME_OVERLAP = 2;


struct FrameData {
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;
    VkSemaphore swapchainSemaphore, renderSemaphore;
    VkFence renderFence;
};

struct Image {
    VkImage image;
    VkImageView view;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
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
	VkExtent2D windowExtent = { 1200, 960 };
	bool bQuit = false;
	void run();

    private:
	int frameNumber = 0;

	GLFWwindow *window;
	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice phyDevice = VK_NULL_HANDLE;

	// Formats
	VkFormat depthFormat;
	VkFormat stencilFormat;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	FrameData frames[FRAME_OVERLAP];
	Image drawImage;

	std::deque<std::function<void()>> deletors;
	void onDestruct(std::function<void()>&& function) {
		deletors.push_back(function); }

	// immediate submit structures
	VkFence immFence;
	VkCommandBuffer immCommandBuffer;
	VkCommandPool immCommandPool;
	void immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function);

	// imGUI
	VkDescriptorPool imguiPool;

	void initWindow();
	void createSurface();
	void initVulkan();
	void initFormats();
	void initSwapchain();
	void initFrameData();
	void initDrawImage();
	void initImCommand();
	void initImgui();
	void draw();

};

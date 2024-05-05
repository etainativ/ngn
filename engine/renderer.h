#pragma once

#include "pipeline.h"
#include "vk_types.h"
#include "mesh.h"
#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include "vk_mem_alloc.h"
#include <vector>
#include <deque>
#include <functional>

constexpr int FRAME_OVERLAP = 2;

struct RendererMeshData;

struct RenderData {
    glm::mat4x4 transform;
    Pipeline::Pipeline* pipeline;
    RendererMeshData* data;
    uint32_t indicesCount;
};

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


class Renderer {
    public:
	Renderer(GLFWwindow *window);
	~Renderer();

	VkDevice device;
	VkSwapchainKHR swapchain;
	VkFormat swapchainImageFormat;

	VkAllocationCallbacks *pAllocator= nullptr;
	VmaAllocator allocator;

	std::vector<VkImage> swapchainImages;
	std::vector<VkImageView> swapchainImageViews;
	VkExtent2D swapchainExtent;
	bool bQuit = false;
	void resize();

	//Mesh
	RendererMeshData* loadMesh(
		std::vector<Mesh::Vertex>& vertices,
		std::vector<uint32_t>& indices);
	void unloadMesh(RendererMeshData *meshData);
	
	//Pipeline
	void loadPipeline(Pipeline::Pipeline& pipeline);
	void unloadPipeline(Pipeline::Pipeline& pipeline);

	//scene
	void draw(std::vector<RenderData>& renderData);

    private:
	int frameNumber = 0;

	VkInstance instance;
	VkDebugUtilsMessengerEXT debugMessenger;
	VkSurfaceKHR surface;
	VkPhysicalDevice phyDevice = VK_NULL_HANDLE;

	GLFWwindow *window;
	// Formats
	VkFormat depthFormat;
	VkFormat stencilFormat;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	FrameData frames[FRAME_OVERLAP];
	Image drawImage;
	Image depthImage;

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

	void createSurface();
	void initVulkan();
	void initFormats();
	void initSwapchain();
	void initFrameData();
	void initImages();
	void initImCommand();
	void initImgui();
	void destroySwapchain();
	void presentImage(
		VkCommandBuffer &cmd,
		FrameData &currFrame,
		uint32_t &swapchainImageIndex);
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

};

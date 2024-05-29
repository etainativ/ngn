#pragma once

#include "engine/glft_object.h"
#include "engine/pipeline.h"
#include "engine/mesh.h"
#include "vk_types.h"
#include <glm/glm.hpp>
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

struct Image {
    VkImage image;
    VkImageView view;
    VmaAllocation allocation;
    VkExtent3D imageExtent;
    VkFormat imageFormat;
};

struct FrameData {
    VkCommandBuffer commandBuffer;
    VkCommandPool commandPool;
    VkSemaphore swapchainSemaphore, renderSemaphore;
    VkFence renderFence;
    Image drawImage;
    Image depthImage;

};

struct DrawContext {
    VkCommandBuffer cmd;
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

	// Drawing
	VkCommandBuffer startDraw();
	void draw(
		VkCommandBuffer cmd,
		Pipeline::Pipeline *pipeline,
		GlftObject::GlftObject *glftObj,
		glm::mat4x4 transform);
	void finishDraw(VkCommandBuffer cmd);

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
	VkFormat imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;

	VkQueue graphicsQueue;
	uint32_t graphicsQueueFamily;

	FrameData frames[FRAME_OVERLAP];
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
	void initDrawImage(Image *image);
	void initDepthImage(Image *image);
	void initImCommand();
	void initImgui();
	void destroySwapchain();
	void presentImage(
		VkCommandBuffer &cmd,
		FrameData &currFrame,
		uint32_t &swapchainImageIndex);
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	// drawing helpers
	void drawNode(
		VkCommandBuffer cmd,
		VkDeviceAddress address,
		Pipeline::Pipeline *pipeline,
		glm::mat4x4 transform,
		GlftObject::GlftNode &node);

};

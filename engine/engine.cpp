#include "engine.h"
#include "engine_utils.h"

#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

//bootstrap library
#include "VkBootstrap.h"

#ifdef NDEBUG
const bool bUseValidationLayers = false;
#else
const bool bUseValidationLayers = true;
#endif


void resizeCallback(GLFWwindow* window, int width, int height);

Engine::~Engine() {
    vkDeviceWaitIdle(device);
    vkDestroySwapchainKHR(device, swapchain, pAllocator);

    // destroy swapchain resources
    for (auto &swapchainImageView : swapchainImageViews)
	    vkDestroyImageView(device, swapchainImageView, pAllocator);

    for (auto frameData: frames)
    {
	vkDestroyCommandPool(device, frameData.commandPool, pAllocator);
	vkDestroyFence(device, frameData.renderFence, pAllocator);
	vkDestroySemaphore(device, frameData.renderSemaphore, pAllocator);
	vkDestroySemaphore(device, frameData.swapchainSemaphore, pAllocator);
    }

    vmaDestroyAllocator(allocator);
    vkDestroyDevice(device, nullptr);
    if (bUseValidationLayers) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
	    vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	func(instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(instance, surface, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}


Engine::Engine() {
    initWindow();
    initVulkan();
    initSwapchain();
}


void Engine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
}

void Engine::initVulkan() {
    vkb::InstanceBuilder builder;

    //make the vulkan instance, with basic debug features
    auto inst_ret = builder.set_app_name("SWE")
	.request_validation_layers(bUseValidationLayers)
	.use_default_debug_messenger()
	.require_api_version(1, 3, 0)
	.build();

    vkb::Instance vkb_inst = inst_ret.value();

    //grab the instance
    instance = vkb_inst.instance;
    debugMessenger = vkb_inst.debug_messenger;

    //create window surface
    createSurface();

    VkPhysicalDeviceVulkan13Features features{};
    features.dynamicRendering = true;
    features.synchronization2 = true;

    //vulkan 1.2 features
    VkPhysicalDeviceVulkan12Features features12{};
    features12.bufferDeviceAddress = true;
    features12.descriptorIndexing = true;

    //use vkbootstrap to select a gpu.
    //We want a gpu that can write to the glfw surface
    //and supports vulkan 1.3 with the correct features
    vkb::PhysicalDeviceSelector selector{ vkb_inst };
    vkb::PhysicalDevice physicalDevice = selector
	.set_minimum_version(1, 3)
	.set_required_features_13(features)
	.set_required_features_12(features12)
	.set_surface(surface)
	.select()
	.value();

    //create the final vulkan device
    vkb::DeviceBuilder deviceBuilder{ physicalDevice };

    vkb::Device vkbDevice = deviceBuilder.build().value();

    // Get the VkDevice handle used in the rest of a vulkan application
    device = vkbDevice.device;
    phyDevice = physicalDevice.physical_device;

    graphicsQueue = vkbDevice.get_queue(vkb::QueueType::graphics).value();
    graphicsQueueFamily = vkbDevice.get_queue_index(vkb::QueueType::graphics).value();

    // initialize the memory allocator
    VmaAllocatorCreateInfo allocatorInfo = {};
    allocatorInfo.physicalDevice = physicalDevice;
    allocatorInfo.device = device;
    allocatorInfo.instance = instance;
    allocatorInfo.flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT;
    vmaCreateAllocator(&allocatorInfo, &allocator);

}

void Engine::createSurface() {
    if (glfwCreateWindowSurface(
		instance,
		window,
		nullptr,
		&surface) != VK_SUCCESS) {
	throw std::runtime_error("failed to create window surface!");
    }
}

void Engine::initSwapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{ phyDevice, device, surface };

    swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    vkb::Swapchain vkbSwapchain = swapchainBuilder
	//.use_default_format_selection()
	.set_desired_format(VkSurfaceFormatKHR{
		.format = swapchainImageFormat,
		.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR })
	//use vsync present mode
	.set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
	.set_desired_extent(width, height)
	.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	.build()
	.value();

    swapchainExtent = vkbSwapchain.extent;
    //store swapchain and its related images
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();
}


void Engine::initFrameData()
{
    //create a command pool for commands submitted to the graphics queue.
    //we also want the pool to allow for resetting of individual command buffers
    VkCommandPoolCreateInfo commandPoolInfo =  {};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.pNext = nullptr;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = graphicsQueueFamily;

    for (auto frame : frames)
    {
	VK_CHECK(vkCreateCommandPool(device, &commandPoolInfo, nullptr, &frame.commandPool));

	// allocate the default command buffer that we will use for rendering
	VkCommandBufferAllocateInfo cmdAllocInfo = {};
	cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cmdAllocInfo.pNext = nullptr;
	cmdAllocInfo.commandPool = frame.commandPool;
	cmdAllocInfo.commandBufferCount = 1;
	cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &frame.commandBuffer));
	
	VkSemaphoreCreateInfo semaforeCI = {};
	semaforeCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaforeCI.pNext = nullptr;
	VK_CHECK(vkCreateSemaphore(device, &semaforeCI, pAllocator, &frame.renderSemaphore));
	VK_CHECK(vkCreateSemaphore(device, &semaforeCI, pAllocator, &frame.swapchainSemaphore));

	VkFenceCreateInfo fenceCI = {};
	fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCI.pNext = nullptr;
	fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK(vkCreateFence(device, &fenceCI, pAllocator, &frame.renderFence));
    }

}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Engine* engine = (Engine *)glfwGetWindowUserPointer(window);
    engine->bQuit = true;
}


void resizeCallback(GLFWwindow* window, int width, int height) {
    Engine *self = (Engine*)glfwGetWindowUserPointer(window);
    self->width = width;
    self->height = height;
}

void Engine::draw()
{
}

void Engine::run()
{
    glfwSetKeyCallback(window, keyCallback);
    //main loop
    while (!bQuit)
    {
	glfwPollEvents();
	draw();
    }
}

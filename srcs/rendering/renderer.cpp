#include "rendering/renderer.h"
#include "engine/pipeline.h"

#include "VkBootstrap.h"
#include "rendering/vulkanInitilizers.h"

#include <glm/fwd.hpp>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

//bootstrap library

// imgui
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

const bool bUseValidationLayers = true;

const VkBool32 wait = 1000000000;

// UTIL
static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

struct RendererMeshData {
	AllocatedBuffer vertexBuffer;
	AllocatedBuffer indicesBuffer;
};

VkBool32 getSupportedDepthFormat(VkPhysicalDevice physicalDevice, VkFormat *depthFormat)
{
    // Since all depth formats may be optional, we need to find a suitable depth format to use
    // Start with the highest precision packed format
    std::vector<VkFormat> formatList = {
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D32_SFLOAT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM
    };

    for (auto& format : formatList)
    {
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
	    *depthFormat = format;
	    return 0;
	}
    }

    return 1;
}

VkBool32 getSupportedDepthStencilFormat(VkPhysicalDevice physicalDevice, VkFormat* depthStencilFormat)
{
    std::vector<VkFormat> formatList = {
	VK_FORMAT_D32_SFLOAT_S8_UINT,
	VK_FORMAT_D24_UNORM_S8_UINT,
	VK_FORMAT_D16_UNORM_S8_UINT,
    };

    for (auto& format : formatList)
    {
	VkFormatProperties formatProps;
	vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
	    *depthStencilFormat = format;
	    return 0;
	}
    }

    return 1;
}

void copy_image_to_image(VkCommandBuffer cmd, VkImage source, VkImage destination, VkExtent2D srcSize, VkExtent2D dstSize)
{
	VkImageBlit2 blitRegion{ .sType = VK_STRUCTURE_TYPE_IMAGE_BLIT_2, .pNext = nullptr };

	blitRegion.srcOffsets[1].x = srcSize.width;
	blitRegion.srcOffsets[1].y = srcSize.height;
	blitRegion.srcOffsets[1].z = 1;

	blitRegion.dstOffsets[1].x = dstSize.width;
	blitRegion.dstOffsets[1].y = dstSize.height;
	blitRegion.dstOffsets[1].z = 1;

	blitRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.srcSubresource.baseArrayLayer = 0;
	blitRegion.srcSubresource.layerCount = 1;
	blitRegion.srcSubresource.mipLevel = 0;

	blitRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	blitRegion.dstSubresource.baseArrayLayer = 0;
	blitRegion.dstSubresource.layerCount = 1;
	blitRegion.dstSubresource.mipLevel = 0;

	VkBlitImageInfo2 blitInfo{ .sType = VK_STRUCTURE_TYPE_BLIT_IMAGE_INFO_2, .pNext = nullptr };
	blitInfo.dstImage = destination;
	blitInfo.dstImageLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	blitInfo.srcImage = source;
	blitInfo.srcImageLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	blitInfo.filter = VK_FILTER_LINEAR;
	blitInfo.regionCount = 1;
	blitInfo.pRegions = &blitRegion;

	vkCmdBlitImage2(cmd, &blitInfo);
}

void transition_image(VkCommandBuffer cmd, VkImage image, VkImageLayout currentLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier2 imageBarrier {.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2};
    imageBarrier.pNext = nullptr;

    imageBarrier.srcStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.srcAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT;
    imageBarrier.dstStageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT;
    imageBarrier.dstAccessMask = VK_ACCESS_2_MEMORY_WRITE_BIT | VK_ACCESS_2_MEMORY_READ_BIT;

    imageBarrier.oldLayout = currentLayout;
    imageBarrier.newLayout = newLayout;

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

    VkImageSubresourceRange subImage {};
    subImage.aspectMask = aspectMask;
    subImage.baseMipLevel = 0;
    subImage.levelCount = VK_REMAINING_MIP_LEVELS;
    subImage.baseArrayLayer = 0;
    subImage.layerCount = VK_REMAINING_ARRAY_LAYERS;

    imageBarrier.subresourceRange = subImage;
    imageBarrier.image = image;

    VkDependencyInfo depInfo {};
    depInfo.sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO;
    depInfo.pNext = nullptr;

    depInfo.imageMemoryBarrierCount = 1;
    depInfo.pImageMemoryBarriers = &imageBarrier;

    vkCmdPipelineBarrier2(cmd, &depInfo);
}

#define VK_CHECK(func) do { if (func) abort(); } while(0);

Renderer::~Renderer() {
    vkDeviceWaitIdle(device);
    destroySwapchain();
    // destroy swapchain resources
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
	(*it)(); //call functors
    }
}


Renderer::Renderer(GLFWwindow *window) {
    this->window = window;
    initVulkan();
    initFormats();
    initSwapchain();
    initFrameData();
    initImCommand();
    initImgui();
}


void Renderer::initVulkan() {
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

    onDestruct([&]() {
	    vmaDestroyAllocator(allocator);
	    vkDestroyDevice(device, nullptr);
	    if (bUseValidationLayers) {
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		func(instance, debugMessenger, nullptr);
	    }
	    vkDestroySurfaceKHR(instance, surface, nullptr);
	    vkDestroyInstance(instance, nullptr);
	});
}


void Renderer::initFormats() {
	VK_CHECK(getSupportedDepthFormat(phyDevice, &depthFormat));
	VK_CHECK(getSupportedDepthStencilFormat(phyDevice, &stencilFormat));
}

void Renderer::createSurface() {
    if (glfwCreateWindowSurface(
		instance,
		window,
		nullptr,
		&surface) != VK_SUCCESS) {
	throw std::runtime_error("failed to create window surface!");
    }
}

void Renderer::initSwapchain()
{
    vkb::SwapchainBuilder swapchainBuilder{ phyDevice, device, surface };

    swapchainImageFormat = VK_FORMAT_B8G8R8A8_UNORM;

    int height, width;
    glfwGetWindowSize(window, &width, &height);

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


void Renderer::destroySwapchain()
{
    vkDestroySwapchainKHR(device, swapchain, pAllocator);
    for (auto &swapchainImageView : swapchainImageViews)
	vkDestroyImageView(device, swapchainImageView, pAllocator);
}


void Renderer::initFrameData()
{
    for (FrameData &frame : frames)
    {
	//create a command pool for commands submitted to the graphics queue.
	//we also want the pool to allow for resetting of individual command buffers
	VkCommandPoolCreateInfo commandPoolInfo = vk::init::commandPoolCreateInfo(graphicsQueueFamily);
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

	onDestruct([&]() { vkDestroySemaphore(device, frame.renderSemaphore, pAllocator);});
	onDestruct([&]() { vkDestroySemaphore(device, frame.swapchainSemaphore, pAllocator);});
	onDestruct([&]() { vkDestroyFence(device, frame.renderFence, pAllocator);});
	onDestruct([&]() { vkDestroyCommandPool(device, frame.commandPool, pAllocator);});

	initDrawImage(&frame.drawImage);
	initDepthImage(&frame.depthImage);
    }

}


void Renderer::initDrawImage(Image *image) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    VkExtent3D drawImageExtent = { (uint32_t)width, (uint32_t)height, 1 };

    //hardcoding the draw format to 32 bit float
    image->imageFormat = imageFormat;
    image->imageExtent = drawImageExtent;

    VkImageCreateInfo imageCI = vk::init::imageCreateInfo();
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = image->imageFormat;
    imageCI.extent = image->imageExtent;
    imageCI.mipLevels = 1;
    imageCI.arrayLayers = 1;

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    imageCI.samples = VK_SAMPLE_COUNT_1_BIT;

    //optimal tiling, which means the image is stored on the best gpu format
    imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageCI.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT
	| VK_IMAGE_USAGE_TRANSFER_DST_BIT
	| VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT // for graphical shader
	| VK_IMAGE_USAGE_STORAGE_BIT; // for compute shader

    //for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //allocate and create the image
    VK_CHECK(vmaCreateImage(allocator, &imageCI, &rimg_allocinfo, &image->image, &image->allocation, nullptr));
    onDestruct([&]() {vmaDestroyImage(allocator, image->image, image->allocation);});

    VkImageViewCreateInfo imageViewCI = vk::init::imageViewCreateInfo();
    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = image->image;
    imageViewCI.format = image->imageFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VK_CHECK(vkCreateImageView(device, &imageViewCI, pAllocator, &image->view));
    onDestruct([=]() { vkDestroyImageView(device, image->view, pAllocator); });
}


void Renderer::initDepthImage(Image *image) {
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    VkExtent3D drawImageExtent = { (uint32_t)width, (uint32_t)height, 1 };

    //hardcoding the draw format to 32 bit float
    image->imageFormat = depthFormat;
    image->imageExtent = drawImageExtent;

    VkImageCreateInfo depthCI = {};
    depthCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    depthCI.pNext = nullptr;
    depthCI.imageType = VK_IMAGE_TYPE_2D;
    depthCI.format = image->imageFormat;
    depthCI.extent = image->imageExtent;
    depthCI.mipLevels = 1;
    depthCI.arrayLayers = 1;

    //for MSAA. we will not be using it by default, so default it to 1 sample per pixel.
    depthCI.samples = VK_SAMPLE_COUNT_1_BIT;

    //for the draw image, we want to allocate it from gpu local memory
    VmaAllocationCreateInfo rimg_allocinfo = {};
    rimg_allocinfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
    rimg_allocinfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    //optimal tiling, which means the image is stored on the best gpu format
    depthCI.tiling = VK_IMAGE_TILING_OPTIMAL;
    depthCI.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    VK_CHECK(vmaCreateImage(allocator, &depthCI, &rimg_allocinfo, &image->image, &image->allocation, nullptr));
    onDestruct([&]() {vmaDestroyImage(allocator, image->image, image->allocation);});

    VkImageViewCreateInfo depthViewCI = vk::init::imageViewCreateInfo();
    depthViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    depthViewCI.subresourceRange.baseMipLevel = 0;
    depthViewCI.subresourceRange.levelCount = 1;
    depthViewCI.subresourceRange.baseArrayLayer = 0;
    depthViewCI.subresourceRange.layerCount = 1;
    depthViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
    depthViewCI.image = image->image;
    depthViewCI.format = image->imageFormat;
    VK_CHECK(vkCreateImageView(device, &depthViewCI, pAllocator, &image->view));
    onDestruct([&]() { vkDestroyImageView(device, image->view, pAllocator); });
}


void Renderer::initImCommand() {
    VkFenceCreateInfo fenceCI = {};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceCI, pAllocator, &immFence);
    onDestruct([&](){vkDestroyFence(device, immFence, pAllocator);});

    VkCommandPoolCreateInfo commandPoolCI = vk::init::commandPoolCreateInfo(graphicsQueueFamily);
    vkCreateCommandPool(device, &commandPoolCI, pAllocator, &immCommandPool);
    onDestruct([&](){vkDestroyCommandPool(device, immCommandPool, pAllocator);});

    VkCommandBufferAllocateInfo cmdAllocInfo = {};
    cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    cmdAllocInfo.pNext = nullptr;
    cmdAllocInfo.commandPool = immCommandPool;
    cmdAllocInfo.commandBufferCount = 1;
    cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    VK_CHECK(vkAllocateCommandBuffers(device, &cmdAllocInfo, &immCommandBuffer));
}


void Renderer::initImgui() {
    // 1: create descriptor pool for IMGUI
    //  the size of the pool is very oversize, but it's copied from imgui demo
    //  itself.
    VkDescriptorPoolSize pool_sizes[] = { { VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
	{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 }};

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 1;
    pool_info.poolSizeCount = (uint32_t)std::size(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;
    VK_CHECK(vkCreateDescriptorPool(device, &pool_info, pAllocator, &imguiPool));
    onDestruct([&](){ vkDestroyDescriptorPool(device, imguiPool, pAllocator); });

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for GLFW
    ImGui_ImplGlfw_InitForVulkan(window, true);

    VkPipelineRenderingCreateInfo pipelineRenderingCI {};
    pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingCI.viewMask = 0;
    pipelineRenderingCI.colorAttachmentCount = 1;
    pipelineRenderingCI.pColorAttachmentFormats = &imageFormat;
    pipelineRenderingCI.depthAttachmentFormat = depthFormat;

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo initInfo = {};
    initInfo.Instance = instance;
    initInfo.PhysicalDevice = phyDevice;
    initInfo.Device = device;
    initInfo.QueueFamily = graphicsQueueFamily;
    initInfo.Queue = graphicsQueue;
    initInfo.DescriptorPool = imguiPool;
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.Allocator = pAllocator;
    initInfo.UseDynamicRendering = true;
    initInfo.PipelineRenderingCreateInfo = pipelineRenderingCI;
    initInfo.CheckVkResultFn = check_vk_result;
    initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&initInfo);

    ImGui_ImplVulkan_CreateFontsTexture();

    // add the destroy the imgui created structures
    onDestruct([&]() {
	    ImGui_ImplVulkan_Shutdown();
	    ImGui_ImplGlfw_Shutdown();
	    ImGui::DestroyContext();});
}

void Renderer::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
    VK_CHECK(vkResetFences(device, 1, &immFence));
    VK_CHECK(vkResetCommandBuffer(immCommandBuffer, 0));

    VkCommandBuffer cmd = immCommandBuffer;

    VkCommandBufferBeginInfo cmdBeginInfo = {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    function(cmd);

    VK_CHECK(vkEndCommandBuffer(cmd));

    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.pNext = nullptr;
    commandBufferSubmitInfo.commandBuffer = cmd;
    commandBufferSubmitInfo.deviceMask = 0;

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreInfoCount = 0;
    submitInfo.pWaitSemaphoreInfos = nullptr;
    submitInfo.signalSemaphoreInfoCount = 0;
    submitInfo.pSignalSemaphoreInfos = nullptr;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;


    // submit command buffer to the queue and execute it.
    //  _renderFence will now block until the graphic commands finish execution
    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, immFence));
    VK_CHECK(vkWaitForFences(device, 1, &immFence, true, wait));
}

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    Renderer* engine = (Renderer *)glfwGetWindowUserPointer(window);
    engine->bQuit = true;
}


void Renderer::resize() {
    destroySwapchain();
    initSwapchain();
}

void Renderer::presentImage(
	VkCommandBuffer &cmd, FrameData &currFrame, uint32_t &swapchainImageIndex)
{
    // SUBMIT
    VkCommandBufferSubmitInfo commandBufferSubmitInfo{};
    commandBufferSubmitInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
    commandBufferSubmitInfo.pNext = nullptr;
    commandBufferSubmitInfo.commandBuffer = cmd;
    commandBufferSubmitInfo.deviceMask = 0;

    VkSemaphoreSubmitInfo waitInfo{};
    waitInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    waitInfo.pNext = nullptr;
    waitInfo.semaphore = currFrame.swapchainSemaphore;
    waitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT_KHR;
    waitInfo.deviceIndex = 0;
    waitInfo.value = 1;

    VkSemaphoreSubmitInfo signalInfo{};
    signalInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
    signalInfo.pNext = nullptr;
    signalInfo.semaphore = currFrame.renderSemaphore;
    signalInfo.stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT;
    signalInfo.deviceIndex = 0;
    signalInfo.value = 1;

    VkSubmitInfo2 submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
    submitInfo.pNext = nullptr;
    submitInfo.waitSemaphoreInfoCount = 1;
    submitInfo.pWaitSemaphoreInfos = &waitInfo;
    submitInfo.signalSemaphoreInfoCount = 1;
    submitInfo.pSignalSemaphoreInfos = &signalInfo;
    submitInfo.commandBufferInfoCount = 1;
    submitInfo.pCommandBufferInfos = &commandBufferSubmitInfo;

    VK_CHECK(vkQueueSubmit2(graphicsQueue, 1, &submitInfo, currFrame.renderFence));
    // present
    VkPresentInfoKHR presentInfo = {};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext = nullptr;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.pWaitSemaphores = &currFrame.renderSemaphore;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pImageIndices = &swapchainImageIndex;

    VK_CHECK(vkQueuePresentKHR(graphicsQueue, &presentInfo));

}


VkCommandBuffer Renderer::startDraw() {
    FrameData& currFrame = frames[frameNumber % FRAME_OVERLAP];
    Image& drawImage = currFrame.drawImage;
    Image& depthImage = currFrame.depthImage;

    VK_CHECK(vkWaitForFences(device, 1, &currFrame.renderFence, true, wait));
    VK_CHECK(vkResetFences(device, 1, &currFrame.renderFence));

    VkCommandBuffer cmd = currFrame.commandBuffer;
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    VkCommandBufferBeginInfo cmdBeginInfo = vk::init::cmdBeginInfo();
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    //make a clear-color from frame number. This will flash with a 120 frame period.
    VkClearColorValue clearValue = { { 0.1f, 0.1f, 0.1f, 1.0f } };

    VkImageSubresourceRange clearRange = vk::init::imageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
    vkCmdClearColorImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    // DRAWING IMGUI
    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = drawImage.view;
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingAttachmentInfo depthAttachment {};
    depthAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    depthAttachment.pNext = nullptr;

    depthAttachment.imageView = depthImage.view;
    depthAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthAttachment.clearValue.depthStencil.depth = 0.f;

    VkExtent2D imageExtent = {
	.width = drawImage.imageExtent.width,
	.height = drawImage.imageExtent.height};

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;
    renderInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, imageExtent };

    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachment;
    renderInfo.pDepthAttachment = &depthAttachment;
    renderInfo.pStencilAttachment = nullptr;

    transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    //transition_image(cmd, depthImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL);
    vkCmdBeginRendering(cmd, &renderInfo);
    return cmd;
}

void Renderer::drawNode(
	VkCommandBuffer cmd,
	VkDeviceAddress address,
	Pipeline::Pipeline *pipeline,
	glm::mat4x4 transform,
	GlftObject::GlftNode &node)
{

    transform = transform * node.transform;
    GPUDrawPushConstants pushConstants = {
	.worldMatrix = transform,
	.vertexBuffer = address
    };

    vkCmdPushConstants(
	    cmd,
	    pipeline->pipelineLayout,
	    VK_SHADER_STAGE_VERTEX_BIT, 0,
	    sizeof(GPUDrawPushConstants), (void*)&pushConstants);
    for (auto mesh : node.meshes)
	vkCmdDrawIndexed(cmd, mesh.indexCount, 1, mesh.firstIndex, 0, 0);
    for (auto child : node.children)
	drawNode(cmd, address, pipeline, transform, child);
}

void Renderer::draw(
	VkCommandBuffer cmd,
	Pipeline::Pipeline *pipeline,
	GlftObject::GlftObject *glftObj,
	glm::mat4x4 transform)
{
    auto meshData = glftObj->meshData;
    FrameData& currFrame = frames[frameNumber % FRAME_OVERLAP];
    Image& drawImage = currFrame.drawImage;
    //set dynamic viewport and scissor
    VkViewport viewport = {};
    viewport.x = 0;
    viewport.y = 0;
    viewport.width = drawImage.imageExtent.width;
    viewport.height = drawImage.imageExtent.height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;

    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = vk::init::createRect2D(drawImage.imageExtent);
    vkCmdSetScissor(cmd, 0, 1, &scissor);
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->vkPipeline);
    vkCmdBindIndexBuffer(cmd, meshData->indicesBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    for (GlftObject::GlftNode &node : glftObj->children) {
	drawNode(cmd, glftObj->meshData->vertexBuffer.address, pipeline, transform, node);
    }
}


void Renderer::finishDraw(VkCommandBuffer cmd) {
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);

    FrameData& currFrame = frames[frameNumber % FRAME_OVERLAP];
    Image& drawImage = currFrame.drawImage;
    VkExtent2D imageExtent = {
	.width = drawImage.imageExtent.width,
	.height = drawImage.imageExtent.height};

    //request image from the swapchain
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, wait, currFrame.swapchainSemaphore, nullptr, &swapchainImageIndex));

    //transition the draw image and the swapchain image into their correct transfer layouts
    transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // execute a copy from the draw image into the swapchain
    copy_image_to_image(cmd, drawImage.image, swapchainImages[swapchainImageIndex], imageExtent, swapchainExtent);

    // set swapchain image layout to Attachment Optimal so we can draw it
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    // set swapchain image layout to Present so we can draw it
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

    presentImage(cmd, currFrame, swapchainImageIndex);


    //increase the number of frames drawn
    frameNumber++;
}

AllocatedBuffer Renderer::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
{
	// allocate buffer
	VkBufferCreateInfo bufferInfo = {.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	bufferInfo.pNext = nullptr;
	bufferInfo.size = allocSize;

	bufferInfo.usage = usage;

	VmaAllocationCreateInfo vmaallocInfo = {};
	vmaallocInfo.usage = memoryUsage;
	vmaallocInfo.flags = VMA_ALLOCATION_CREATE_MAPPED_BIT;
	AllocatedBuffer newBuffer;

	// allocate the buffer
	VK_CHECK(vmaCreateBuffer(allocator, &bufferInfo, &vmaallocInfo, &newBuffer.buffer, &newBuffer.allocation,
		&newBuffer.info));

	return newBuffer;
}

RendererMeshData* Renderer::loadMesh(
	std::vector<Mesh::Vertex>& vertices,
	std::vector<uint32_t>& indices
    ) {

    RendererMeshData* mesh = new RendererMeshData();
    size_t vertexBufferSize = vertices.size() * sizeof(Mesh::Vertex);
    size_t indicesBufferSize = indices.size() * sizeof(uint32_t);

    mesh->vertexBuffer = create_buffer(
	    vertexBufferSize,
	    VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
	    VMA_MEMORY_USAGE_GPU_ONLY);

    VkBufferDeviceAddressInfo deviceAdressInfo{
	    .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
	    .pNext = nullptr,
	    .buffer = mesh->vertexBuffer.buffer};

    mesh->vertexBuffer.address = vkGetBufferDeviceAddress(device, &deviceAdressInfo);

    //create index buffer
    mesh->indicesBuffer = create_buffer(
	    indicesBufferSize,
	    VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,	    VMA_MEMORY_USAGE_GPU_ONLY);

    AllocatedBuffer staging = create_buffer(
	    vertexBufferSize + indicesBufferSize,
	    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
	    VMA_MEMORY_USAGE_CPU_ONLY);

    void* data = staging.allocation->GetMappedData();

    // copy vertex buffer
    memcpy(data, vertices.data(), vertexBufferSize);
    // copy index buffer
    memcpy((char*)data + vertexBufferSize, indices.data(), indicesBufferSize);

    immediateSubmit([&](VkCommandBuffer cmd) {
	    VkBufferCopy vertexCopy{};
	    vertexCopy.dstOffset = 0;
	    vertexCopy.srcOffset = 0;
	    vertexCopy.size = vertexBufferSize;

	    vkCmdCopyBuffer(cmd, staging.buffer, mesh->vertexBuffer.buffer, 1, &vertexCopy);

	    VkBufferCopy indexCopy{};
	    indexCopy.dstOffset = 0;
	    indexCopy.srcOffset = vertexBufferSize;
	    indexCopy.size = indicesBufferSize;

	    vkCmdCopyBuffer(cmd, staging.buffer, mesh->indicesBuffer.buffer, 1, &indexCopy);
	    });

    vmaDestroyBuffer(allocator, staging.buffer, staging.allocation);
    return mesh;
}


void Renderer::unloadMesh(RendererMeshData* mesh) {
    vmaDestroyBuffer(allocator, mesh->vertexBuffer.buffer, mesh->vertexBuffer.allocation);
    vmaDestroyBuffer(allocator, mesh->indicesBuffer.buffer, mesh->indicesBuffer.allocation);
    delete mesh;
}

void Renderer::loadPipeline(Pipeline::Pipeline& pipeline) {
    Pipeline::setColorAttachment(pipeline, imageFormat);
    pipeline.renderInfo.depthAttachmentFormat = depthFormat;
    Pipeline::createPipeline(pipeline, device, pAllocator);
}

void Renderer::unloadPipeline(Pipeline::Pipeline& pipeline){
    for (auto &frame : frames) {
	VK_CHECK(vkWaitForFences(device, 1, &frame.renderFence, true, wait));
    }

    Pipeline::destroyPipeline(pipeline, device, pAllocator);
}

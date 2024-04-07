#include "engine.h"

#include <cstdint>
#include <glm/common.hpp>
#include <glm/glm.hpp>
#include <vulkan/vulkan_core.h>

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

//bootstrap library
#include "VkBootstrap.h"

// imgui
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"

#ifdef NDEBUG
const bool bUseValidationLayers = false;
#else
const bool bUseValidationLayers = true;
#endif

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

    VkImageAspectFlags aspectMask = (newLayout == VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

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

void resizeCallback(GLFWwindow* window, int width, int height);

Engine::~Engine() {
    vkDeviceWaitIdle(device);
    // destroy swapchain resources
    for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
	(*it)(); //call functors
    }
}


Engine::Engine() {
    initWindow();
    initVulkan();
    initFormats();
    initSwapchain();
    initFrameData();
    initDrawImage();
    initImCommand();
    initImgui();
}


void Engine::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window = glfwCreateWindow(
	    windowExtent.width, windowExtent.height, "Vulkan", nullptr, nullptr);
    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, resizeCallback);
    onDestruct([&]() {
	    glfwDestroyWindow(window);
	    glfwTerminate();});
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


void Engine::initFormats() {
	VK_CHECK(getSupportedDepthFormat(phyDevice, &depthFormat));
	VK_CHECK(getSupportedDepthStencilFormat(phyDevice, &stencilFormat));
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
	.set_desired_extent(windowExtent.width, windowExtent.height)
	.add_image_usage_flags(VK_IMAGE_USAGE_TRANSFER_DST_BIT)
	.build()
	.value();

    swapchainExtent = vkbSwapchain.extent;
    //store swapchain and its related images
    swapchain = vkbSwapchain.swapchain;
    swapchainImages = vkbSwapchain.get_images().value();
    swapchainImageViews = vkbSwapchain.get_image_views().value();

    onDestruct([&]() {vkDestroySwapchainKHR(device, swapchain, pAllocator);});
    for (auto &swapchainImageView : swapchainImageViews)
	onDestruct([&]() {vkDestroyImageView(device, swapchainImageView, pAllocator);});
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

    for (FrameData &frame : frames)
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

	onDestruct([&]() { vkDestroySemaphore(device, frame.renderSemaphore, pAllocator);});
	onDestruct([&]() { vkDestroySemaphore(device, frame.swapchainSemaphore, pAllocator);});
	onDestruct([&]() { vkDestroyFence(device, frame.renderFence, pAllocator);});
	onDestruct([&]() { vkDestroyCommandPool(device, frame.commandPool, pAllocator);});
    }

}


void Engine::initDrawImage() {
    VkExtent3D drawImageExtent = {
	windowExtent.width,
	windowExtent.height,
	1
    };

    //hardcoding the draw format to 32 bit float
    drawImage.imageFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
    drawImage.imageExtent = drawImageExtent;

    VkImageCreateInfo imageCI = {};
    imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCI.pNext = nullptr;
    imageCI.imageType = VK_IMAGE_TYPE_2D;
    imageCI.format = drawImage.imageFormat;
    imageCI.extent = drawImage.imageExtent;
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
    vmaCreateImage(allocator, &imageCI, &rimg_allocinfo, &drawImage.image, &drawImage.allocation, nullptr);
    onDestruct([&]() {vmaDestroyImage(allocator, drawImage.image, drawImage.allocation);});

    VkImageViewCreateInfo imageViewCI = {};
    imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageViewCI.pNext = nullptr;

    imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageViewCI.image = drawImage.image;
    imageViewCI.format = drawImage.imageFormat;
    imageViewCI.subresourceRange.baseMipLevel = 0;
    imageViewCI.subresourceRange.levelCount = 1;
    imageViewCI.subresourceRange.baseArrayLayer = 0;
    imageViewCI.subresourceRange.layerCount = 1;
    imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VK_CHECK(vkCreateImageView(device, &imageViewCI, pAllocator, &drawImage.view));
    onDestruct([=]() { vkDestroyImageView(device, drawImage.view, pAllocator); });
}


void Engine::initImCommand() {
    VkFenceCreateInfo fenceCI = {};
    fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCI.pNext = nullptr;
    fenceCI.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(device, &fenceCI, pAllocator, &immFence);
    onDestruct([&](){vkDestroyFence(device, immFence, pAllocator);});

    VkCommandPoolCreateInfo commandPoolCI = {};
    commandPoolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolCI.queueFamilyIndex = graphicsQueueFamily;
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


void Engine::initImgui() {
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

    VkPipelineRenderingCreateInfoKHR pipelineRenderingCI {};
    pipelineRenderingCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR;
    pipelineRenderingCI.viewMask = 0;
    pipelineRenderingCI.colorAttachmentCount = 1;
    pipelineRenderingCI.pColorAttachmentFormats = &swapchainImageFormat;
    pipelineRenderingCI.depthAttachmentFormat = depthFormat;
    pipelineRenderingCI.stencilAttachmentFormat = stencilFormat;

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

    //ImGui_ImplVulkan_DestroyFontsTexture();

    // add the destroy the imgui created structures
    onDestruct([&]() { 
	    ImGui_ImplVulkan_Shutdown();
	    ImGui_ImplGlfw_Shutdown();
	    ImGui::DestroyContext();});
}

void Engine::immediateSubmit(std::function<void(VkCommandBuffer cmd)>&& function) {
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
    Engine* engine = (Engine *)glfwGetWindowUserPointer(window);
    engine->bQuit = true;
}


void resizeCallback(GLFWwindow* window, int width, int height) {
    Engine *self = (Engine*)glfwGetWindowUserPointer(window);
    self->windowExtent.width = width;
    self->windowExtent.height = height;
}

void Engine::draw()
{
    FrameData& currFrame = frames[frameNumber % FRAME_OVERLAP];

    VK_CHECK(vkWaitForFences(device, 1, &currFrame.renderFence, true, wait));
    VK_CHECK(vkResetFences(device, 1, &currFrame.renderFence));

    //request image from the swapchain
    uint32_t swapchainImageIndex;
    VK_CHECK(vkAcquireNextImageKHR(device, swapchain, wait, currFrame.swapchainSemaphore, nullptr, &swapchainImageIndex));

    //naming it cmd for shorter writing
    VkCommandBuffer cmd = currFrame.commandBuffer;

    // now that we are sure that the commands finished executing, we can safely
    // reset the command buffer to begin recording again.
    VK_CHECK(vkResetCommandBuffer(cmd, 0));

    //begin the command buffer recording. We will use this command buffer exactly once, so we want to let vulkan know that
    VkCommandBufferBeginInfo cmdBeginInfo {};
    cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmdBeginInfo.pNext = nullptr;
    cmdBeginInfo.pInheritanceInfo = nullptr;
    cmdBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    //start the command buffer recording
    VK_CHECK(vkBeginCommandBuffer(cmd, &cmdBeginInfo));

    // transition our main draw image into general layout so we can write into it
    // we will overwrite it all so we dont care about what was the older layout
    transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_GENERAL);

    //make a clear-color from frame number. This will flash with a 120 frame period.
    VkClearColorValue clearValue;
    float flash = glm::abs(sin(frameNumber / 120.f));
    clearValue = { { 0.0f, 0.0f, flash, 1.0f } };

    VkImageSubresourceRange clearRange{};
    clearRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    clearRange.baseMipLevel = 0;
    clearRange.levelCount = VK_REMAINING_MIP_LEVELS;
    clearRange.baseArrayLayer = 0;
    clearRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

    //clear image
    vkCmdClearColorImage(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, &clearValue, 1, &clearRange);

    //transition the draw image and the swapchain image into their correct transfer layouts
    transition_image(cmd, drawImage.image, VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    // execute a copy from the draw image into the swapchain
    VkExtent2D imageExtent = {
	.width = drawImage.imageExtent.width,
	.height = drawImage.imageExtent.height};
    copy_image_to_image(cmd, drawImage.image, swapchainImages[swapchainImageIndex], imageExtent, swapchainExtent);


    // set swapchain image layout to Attachment Optimal so we can draw it
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);

    // DRAWING IMGUI
    //VkRenderingAttachmentInfo colorAttachment = vkinit::attachment_info(targetImageView, nullptr, VK_IMAGE_LAYOUT_GENERAL);
    VkRenderingAttachmentInfo colorAttachment {};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    colorAttachment.pNext = nullptr;

    colorAttachment.imageView = swapchainImageViews[swapchainImageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    VkRenderingInfo renderInfo = {};
    renderInfo.sType = VK_STRUCTURE_TYPE_RENDERING_INFO;
    renderInfo.pNext = nullptr;
    renderInfo.renderArea = VkRect2D { VkOffset2D { 0, 0 }, swapchainExtent };

    renderInfo.layerCount = 1;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachments = &colorAttachment;
    renderInfo.pDepthAttachment = nullptr;
    renderInfo.pStencilAttachment = nullptr;

    vkCmdBeginRendering(cmd, &renderInfo);

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);

    vkCmdEndRendering(cmd);

    // set swapchain image layout to Present so we can draw it
    transition_image(cmd, swapchainImages[swapchainImageIndex], VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);

    //finalize the command buffer (we can no longer add commands, but it can now be executed)
    VK_CHECK(vkEndCommandBuffer(cmd));

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

    //increase the number of frames drawn
    frameNumber++;
}


void Engine::run()
{
    bool showDemoWindow = false;
    glfwSetKeyCallback(window, keyCallback);
    //main loop
    while (!bQuit)
    {
	glfwPollEvents();
	// imgui new frame
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	//some imgui UI to test
	ImGui::ShowDemoWindow(&showDemoWindow);

	//make imgui calculate internal draw structures
	ImGui::Render();
	draw();
    }
}

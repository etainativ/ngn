#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

namespace Pipeline {
    struct Pipeline {
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
	VkPipelineRasterizationStateCreateInfo rasterizer = {};
	VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
	VkPipelineMultisampleStateCreateInfo multisampling = {};
	VkPipelineLayoutCreateInfo pipelineLayoutCI {};
	VkPipelineDepthStencilStateCreateInfo depthStencil = {};
	VkPipelineRenderingCreateInfo renderInfo = {};
	VkPipelineViewportStateCreateInfo viewportState = {};
	VkPipelineColorBlendStateCreateInfo colorBlending = {};
	VkGraphicsPipelineCreateInfo pipelineCI = {};
	VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
	VkPushConstantRange bufferRange = {};
	VkFormat colorAttachmentformat;
	VkPipelineLayout pipelineLayout;
	VkPipeline vkPipeline;

	const char *vertexShaderFP;
	const char *fragShaderFP;

	std::vector<VkPipelineShaderStageCreateInfo> stages;
    };

    Pipeline initPipelineStruct(
	    const char *vertextShader,
	    const char *fragShaderFP);

    VkPipeline createPipeline(
	    Pipeline &pipeline,
	    VkDevice device,
	    VkAllocationCallbacks* pAllocator);

    void destroyPipeline(
	    Pipeline &pipeline,
	    VkDevice device,
	    VkAllocationCallbacks* pAllocator);

    void setColorAttachment(Pipeline &pipeline, VkFormat format);

}

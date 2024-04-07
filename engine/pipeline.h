#pragma once
#include <vector>
#include <vulkan/vulkan_core.h>

class Pipeline {
    public:
	Pipeline();
	~Pipeline();
	VkPipeline createPipeline(VkDevice device, VkAllocationCallbacks* pAllocator);
	void destroyPipeline(VkDevice device, VkAllocationCallbacks* pAllocator);
	void setColorAttachment(VkFormat format, VkDevice device, VkAllocationCallbacks* pAllocator);
	void setColorAttachment(VkFormat format);

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
	VkPipeline vkPipeline;
	VkPipelineLayout pipelineLayout;
	const char *vertexShaderFP;
	const char *fragShaderFP;

    private:
	VkShaderModule vertextShader;
	VkShaderModule fragShader;
	VkFormat colorAttachmentformat;
	std::vector<VkPipelineShaderStageCreateInfo> stages;
	void addShader(
		const char *shaderFP,
		VkShaderStageFlagBits flags,
		VkShaderModule &shaderModule, VkDevice device, VkAllocationCallbacks* pAllocator);
};

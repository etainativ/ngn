#include <vector>
#include <vulkan/vulkan_core.h>

class Pipeline {
    public:
	Pipeline();
	~Pipeline();
	VkPipeline createPipeline(VkDevice device, VkAllocationCallbacks* pAllocator);
	void addVertexShader(const char *shaderModuleFilePath, VkDevice device, VkAllocationCallbacks* pAllocator);
	void addFragShader(const char *shaderModuleFilePath, VkDevice device, VkAllocationCallbacks* pAllocator);
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

    private:
	VkPipelineLayout pipelineLayout;
	VkPipeline vkPipeline;
	VkShaderModule vertextShader;
	VkShaderModule fragShader;
	std::vector<VkPipelineShaderStageCreateInfo> stages;
	void addShader(
		const char *shaderFP,
		VkShaderStageFlagBits flags,
		VkShaderModule &shaderModule, VkDevice device, VkAllocationCallbacks* pAllocator);
};


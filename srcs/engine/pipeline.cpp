#include "engine/pipeline.h"
#include "utils.h"
#include "vk_types.h"

#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdlib.h>

namespace Pipeline {
    Pipeline initPipelineStruct(
	    const char *vertexShaderFP,
	    const char *fragShaderFP)
    {
	Pipeline pipeline;
	// BASIC INIT
	pipeline.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	pipeline.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	pipeline.inputAssembly.primitiveRestartEnable = VK_FALSE;

	pipeline.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	pipeline.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	pipeline.rasterizer.lineWidth = 1.0f;
	pipeline.rasterizer.cullMode = VK_CULL_MODE_NONE;
	pipeline.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

	// no blending
	pipeline.colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	pipeline.colorBlendAttachment.blendEnable = VK_FALSE;

	pipeline.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	pipeline.multisampling.sampleShadingEnable = VK_FALSE;
	pipeline.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	pipeline.multisampling.minSampleShading = 1.0f;
	pipeline.multisampling.pSampleMask = nullptr;
	pipeline.multisampling.alphaToCoverageEnable = VK_FALSE;
	pipeline.multisampling.alphaToOneEnable = VK_FALSE;

	pipeline.bufferRange = {};
	pipeline.bufferRange.offset = 0;
	pipeline.bufferRange.size = sizeof(GPUDrawPushConstants);
	pipeline.bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipeline.pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline.pipelineLayoutCI.pNext = nullptr;
	pipeline.pipelineLayoutCI.flags = 0;
	pipeline.pipelineLayoutCI.setLayoutCount = 0;
	pipeline.pipelineLayoutCI.pSetLayouts = nullptr;
	pipeline.pipelineLayoutCI.pushConstantRangeCount = 1;
	pipeline.pipelineLayoutCI.pPushConstantRanges = &pipeline.bufferRange;

	pipeline.depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	pipeline.depthStencil.depthTestEnable = VK_FALSE;
	pipeline.depthStencil.depthWriteEnable = true;
	pipeline.depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
	pipeline.depthStencil.depthBoundsTestEnable = VK_FALSE;
	pipeline.depthStencil.stencilTestEnable = VK_FALSE;
	pipeline.depthStencil.front = {};
	pipeline.depthStencil.back = {};
	pipeline.depthStencil.minDepthBounds = 0.f;
	pipeline.depthStencil.maxDepthBounds = 1.f;

	pipeline.renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
	pipeline.renderInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

	pipeline.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	pipeline.viewportState.pNext = nullptr;
	pipeline.viewportState.viewportCount = 1;
	pipeline.viewportState.scissorCount =1;

	pipeline.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	pipeline.colorBlending.pNext = nullptr;
	pipeline.colorBlending.logicOpEnable = VK_FALSE;
	pipeline.colorBlending.logicOp = VK_LOGIC_OP_COPY;
	pipeline.colorBlending.attachmentCount = 1;
	pipeline.colorBlending.pAttachments = &pipeline.colorBlendAttachment;

	pipeline.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	pipeline.vertexShaderFP = vertexShaderFP;
	pipeline.fragShaderFP = fragShaderFP;
	return pipeline;
    };


    void addShader(
	    std::vector<VkPipelineShaderStageCreateInfo> &stages,
	    const char *shaderFP,
	    VkShaderStageFlagBits flags,
	    VkDevice device,
	    VkAllocationCallbacks* pAllocator) {
	// open the file. With cursor at the end
	auto buffer = utils::readFile(shaderFP);
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo shaderCI = {};
	shaderCI.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderCI.codeSize = buffer.size() * sizeof(uint32_t);
	shaderCI.pCode = buffer.data();

	if (vkCreateShaderModule(device, &shaderCI, pAllocator, &shaderModule) != VK_SUCCESS)
	    abort();

	VkPipelineShaderStageCreateInfo info {};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	info.pNext = nullptr;
	info.stage = flags;
	// module containing the code for this shader stage
	info.module = shaderModule;
	info.pName = "main";
	stages.push_back(info);
    }


    void setColorAttachment(Pipeline &pipeline, VkFormat format)
    {
	pipeline.colorAttachmentformat = format;
	pipeline.renderInfo.colorAttachmentCount = 1;
	pipeline.renderInfo.pColorAttachmentFormats = &pipeline.colorAttachmentformat;
    }

    VkPipeline createPipeline(
	    Pipeline& pipeline,
	    VkDevice device,
	    VkAllocationCallbacks* pAllocator)
    {
	pipeline.pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline.pipelineCI.pNext = &pipeline.renderInfo;
	pipeline.pipelineCI.pVertexInputState = &pipeline.vertexInputInfo;
	pipeline.pipelineCI.pInputAssemblyState = &pipeline.inputAssembly;
	pipeline.pipelineCI.pViewportState = &pipeline.viewportState;
	pipeline.pipelineCI.pRasterizationState = &pipeline.rasterizer;
	pipeline.pipelineCI.pMultisampleState = &pipeline.multisampling;
	pipeline.pipelineCI.pColorBlendState = &pipeline.colorBlending;
	pipeline.pipelineCI.pDepthStencilState = &pipeline.depthStencil;

	addShader(
		pipeline.stages, pipeline.vertexShaderFP,
		VK_SHADER_STAGE_VERTEX_BIT, device, pAllocator);
	addShader(
		pipeline.stages, pipeline.fragShaderFP,
		VK_SHADER_STAGE_FRAGMENT_BIT, device, pAllocator);

	vkCreatePipelineLayout(
		device,
		&pipeline.pipelineLayoutCI,
		pAllocator,
		&pipeline.pipelineLayout);

	pipeline.pipelineCI.stageCount = (uint32_t)pipeline.stages.size();
	pipeline.pipelineCI.pStages = pipeline.stages.data();
	pipeline.pipelineCI.layout = pipeline.pipelineLayout;

	VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
	VkPipelineDynamicStateCreateInfo dynamicInfo {};
	dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicInfo.pDynamicStates = &state[0];
	dynamicInfo.dynamicStateCount = 2;

	pipeline.pipelineCI.pDynamicState = &dynamicInfo;

	if (vkCreateGraphicsPipelines(
		    device, VK_NULL_HANDLE, 1,
		    &pipeline.pipelineCI, pAllocator, &pipeline.vkPipeline)
		!= VK_SUCCESS) {
	    return VK_NULL_HANDLE; // failed to create graphics pipeline
	}

	for (auto shader : pipeline.stages)
	    vkDestroyShaderModule(device, shader.module, pAllocator);

	return pipeline.vkPipeline;
    }

    void destroyPipeline(
	    Pipeline& pipeline,
	    VkDevice device,
	    VkAllocationCallbacks* pAllocator)
    {
	vkDestroyPipeline(device, pipeline.vkPipeline, pAllocator);
	vkDestroyPipelineLayout(device, pipeline.pipelineLayout, pAllocator);
    }
}


#include "pipeline.h"
#include "utils.h"
#include "vk_types.h"

#include <vulkan/vulkan_core.h>
#include <vector>
#include <stdlib.h>

Pipeline::~Pipeline() {}
Pipeline::Pipeline() {
    // BASIC INIT
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;

    // no blending
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;
    multisampling.pSampleMask = nullptr;
    multisampling.alphaToCoverageEnable = VK_FALSE;
    multisampling.alphaToOneEnable = VK_FALSE;

    bufferRange = {};
    bufferRange.offset = 0;
    bufferRange.size = sizeof(GPUDrawPushConstants);
    bufferRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

    pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutCI.pNext = nullptr;
    pipelineLayoutCI.flags = 0;
    pipelineLayoutCI.setLayoutCount = 0;
    pipelineLayoutCI.pSetLayouts = nullptr;
    pipelineLayoutCI.pushConstantRangeCount = 1;
    pipelineLayoutCI.pPushConstantRanges = &bufferRange;

    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = true;
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {};
    depthStencil.back = {};
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;

    renderInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO;
    renderInfo.depthAttachmentFormat = VK_FORMAT_D32_SFLOAT;

    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.pNext = nullptr;
    viewportState.viewportCount = 1;
    viewportState.scissorCount =1;

    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.pNext = nullptr;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;


    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineCI.pNext = &renderInfo;
    pipelineCI.pVertexInputState = &vertexInputInfo;
    pipelineCI.pInputAssemblyState = &inputAssembly;
    pipelineCI.pViewportState = &viewportState;
    pipelineCI.pRasterizationState = &rasterizer;
    pipelineCI.pMultisampleState = &multisampling;
    pipelineCI.pColorBlendState = &colorBlending;
    pipelineCI.pDepthStencilState = &depthStencil;
};


void Pipeline::addShader(
	const char *shaderFP,
	VkShaderStageFlagBits flags,
	VkShaderModule &shaderModule,
	VkDevice device,
	VkAllocationCallbacks* pAllocator) {
    // open the file. With cursor at the end
    auto buffer = utils::readFile(shaderFP);
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


void Pipeline::setColorAttachment(VkFormat format)
{
    colorAttachmentformat = format;
    renderInfo.colorAttachmentCount = 1;
    renderInfo.pColorAttachmentFormats = &colorAttachmentformat;
}

VkPipeline Pipeline::createPipeline(VkDevice device, VkAllocationCallbacks* pAllocator)
{
    addShader(vertexShaderFP, VK_SHADER_STAGE_VERTEX_BIT, vertextShader, device, pAllocator);
    addShader(fragShaderFP, VK_SHADER_STAGE_FRAGMENT_BIT, fragShader, device, pAllocator);
    vkCreatePipelineLayout(device, &pipelineLayoutCI, pAllocator, &pipelineLayout);

    pipelineCI.stageCount = (uint32_t)stages.size();
    pipelineCI.pStages = stages.data();
    pipelineCI.layout = pipelineLayout;

    VkDynamicState state[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicInfo {};
    dynamicInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicInfo.pDynamicStates = &state[0];
    dynamicInfo.dynamicStateCount = 2;

    pipelineCI.pDynamicState = &dynamicInfo;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineCI, pAllocator, &vkPipeline)
	    != VK_SUCCESS) {
	return VK_NULL_HANDLE; // failed to create graphics pipeline
    }

    for (auto shader : stages)
	vkDestroyShaderModule(device, shader.module, pAllocator);

    return vkPipeline;
}

void Pipeline::destroyPipeline(VkDevice device, VkAllocationCallbacks* pAllocator)
{
    vkDestroyPipeline(device, vkPipeline, pAllocator);
    vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

#include "app-base.h"
#include "pipeline-resource.h"
#include "vertex.h"
#include "app-config.h"

void AppPipeline::init(AppBase* appBase, std::vector<AppShaderModule> shaderModules, AppPipelineLayout pipelineLayout, AppRenderPass renderPass)
{
    // Configure vertex buffer binding

    VkVertexInputBindingDescription vertBindDesc{};
    vertBindDesc.binding = 0;
    vertBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertBindDesc.stride = sizeof(Vertex);

    // Get the attribute description for the Vertex structure
    std::vector<VkVertexInputAttributeDescription> description = Vertex::getAttributeDescriptions();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertBindDesc; 
    vertexInputInfo.vertexAttributeDescriptionCount = description.size();
    vertexInputInfo.pVertexAttributeDescriptions = description.data();

    // Configure some input assembler setup
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Configure the viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) appBase->viewportSettings.width;
    viewport.height = (float) appBase->viewportSettings.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Configure the scissor (can be used to discard rasterizer pixels)
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = appBase->viewportSettings.width;
    scissor.extent.height = appBase->viewportSettings.height;

    // Since we are dynamically specifying the viewport and scissor struct, theres no need to specify them in the viewport
    // state. We will set these values up later at draw time.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;
    viewportState.pViewports = &viewport;
    viewportState.flags = 0U;
    viewportState.pNext = nullptr;

    // Set up some rasterizer values
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    // Configure some multisampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    // Configure the blend attachment state
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    // Configure colour blending
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional


    std::vector<VkPipelineShaderStageCreateInfo> shaderStageCreateInfos = {};
    for (AppShaderModule appShaderModule : shaderModules) {
        shaderStageCreateInfos.push_back(VkPipelineShaderStageCreateInfo{});
        shaderStageCreateInfos.back().sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageCreateInfos.back().pNext = nullptr;
        shaderStageCreateInfos.back().flags = 0U;
        shaderStageCreateInfos.back().module = appShaderModule.get();
        shaderStageCreateInfos.back().pName = "main";
        shaderStageCreateInfos.back().stage = appShaderModule.getShaderStage();
        shaderStageCreateInfos.back().pSpecializationInfo = nullptr;
    }

    VkPipelineDepthStencilStateCreateInfo dsStateCreateInfo{};
    dsStateCreateInfo.pNext = nullptr;
    dsStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    dsStateCreateInfo.depthTestEnable = true;
    dsStateCreateInfo.stencilTestEnable = false;
    dsStateCreateInfo.maxDepthBounds = 1.f;
    dsStateCreateInfo.minDepthBounds = 0.f;
    dsStateCreateInfo.depthWriteEnable = true;
    dsStateCreateInfo.depthCompareOp = VK_COMPARE_OP_LESS;

    VkGraphicsPipelineCreateInfo colorSubpassPipelineInfo{};
    colorSubpassPipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    colorSubpassPipelineInfo.stageCount = shaderStageCreateInfos.size();
    colorSubpassPipelineInfo.pStages = shaderStageCreateInfos.data();
    colorSubpassPipelineInfo.pVertexInputState = &vertexInputInfo;
    colorSubpassPipelineInfo.pInputAssemblyState = &inputAssembly;
    colorSubpassPipelineInfo.pViewportState = &viewportState;
    colorSubpassPipelineInfo.pRasterizationState = &rasterizer;
    colorSubpassPipelineInfo.pMultisampleState = &multisampling;
    colorSubpassPipelineInfo.pDepthStencilState = &dsStateCreateInfo; // Optional
    colorSubpassPipelineInfo.pColorBlendState = &colorBlending;
    colorSubpassPipelineInfo.pDynamicState = nullptr;
    colorSubpassPipelineInfo.layout = pipelineLayout.get();
    colorSubpassPipelineInfo.renderPass = renderPass.get();
    colorSubpassPipelineInfo.subpass = 0;
    colorSubpassPipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    colorSubpassPipelineInfo.basePipelineIndex = -1; // Optional
    
    VkPipeline pipeline = VK_NULL_HANDLE;
    THROW(vkCreateGraphicsPipelines(appBase->getDevice(), VK_NULL_HANDLE, 1, &colorSubpassPipelineInfo, NULL, &pipeline), "Failed to create graphics pipeline");

    AppResource::init(appBase, appBase->resources.pipelines.create(pipeline));
}

void AppPipeline::destroy()
{
    appBase->resources.pipelines.destroy(getIterator(), appBase->getDevice());
}

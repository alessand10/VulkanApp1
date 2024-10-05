#include "vulkan-app.h"
#include "pipeline-resource.h"
#include "vertex.h"

void AppPipeline::init(VulkanApp *app, std::vector<AppShaderModule> shaderModules, AppPipelineLayout pipelineLayout, AppRenderPass renderPass)
{
    // Configure vertex buffer binding

    VkVertexInputBindingDescription vertBindDesc{};
    vertBindDesc.binding = 0;
    vertBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertBindDesc.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription attributeDesc[4] = {
        // Position
        {
            0U, // Location, the shader input location (layout(location = 0) in vec3 position)
            0U, // Binding, the binding number of the vertex buffer from which this data is coming,
            VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the position,
            0U // Offset, this is the first attribute so use an offset of 0
        },

        // Normal
        {
            1U, // Location, the shader input location (layout(location = 1) in vec3 normal)
            0U, // Binding, the binding number of the vertex buffer from which this data is coming,
            VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the normal,
            12U // Offset, we use 12 bytes since position is made up of 3 x 4-byte values
        },
        // Tangent
        {
            2U, // Location, the shader input location (layout(location = 1) in vec3 normal)
            0U, // Binding, the binding number of the vertex buffer from which this data is coming,
            VK_FORMAT_R32G32B32_SFLOAT, // Format, we use a Float3 for the normal,
            24U // Offset, we use 12 bytes since position is made up of 3 x 4-byte values
        },

        // Texcoord
        {
            3U, // Location, the shader input location (layout(location = 2) in vec2 texCoord)
            0U, // Binding, the binding number of the vertex buffer from which this data is coming,
            VK_FORMAT_R32G32_SFLOAT, // Format, we use a Float2 for the texture coordinate,
            36U // Offset, we use 24 bytes since position & normal take up 6 x 4-byte values
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertBindDesc; 
    vertexInputInfo.vertexAttributeDescriptionCount = 4;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDesc;

    // Configure some input assembler setup
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Configure the viewport
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) app->viewportSettings.width;
    viewport.height = (float) app->viewportSettings.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Configure the scissor (can be used to discard rasterizer pixels)
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = app->viewportSettings.width;
    scissor.extent.height = app->viewportSettings.height;

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
    THROW(vkCreateGraphicsPipelines(app->logicalDevice.get(), VK_NULL_HANDLE, 1, &colorSubpassPipelineInfo, NULL, &pipeline), "Failed to create graphics pipeline");

    AppResource::init(app, app->resources.pipelines.create(pipeline));
}

void AppPipeline::destroy()
{
    getApp()->resources.pipelines.destroy(getIterator(), getApp()->logicalDevice.get());
}

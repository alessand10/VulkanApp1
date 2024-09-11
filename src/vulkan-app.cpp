#include "vulkan-app.h"
#include <iostream>
#include "GLFW/glfw3.h"
#include <fstream>
#include <mesh.h>
#include <attach.h>
#include <utilities.h>

VulkanApp* appHandle;

bool VulkanApp::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    return true;
}

std::vector<char> VulkanApp::readFile(const std::string filename)
{
    std::vector<char> fileData;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    bool open = file.is_open();
    size_t fileSize = (size_t)file.tellg();
    fileData.resize(fileSize);
    file.seekg(0);
    file.read(fileData.data(), fileSize);
    return fileData;
}


VKAPI_ATTR VkBool32 VKAPI_CALL VulkanApp::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData, void *pUserData)
{
    std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

    return VK_FALSE;

}

VkResult VulkanApp::CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)

{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void VulkanApp::DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

void VulkanApp::setupDebugMessenger()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Optional

    if (CreateDebugUtilsMessengerEXT(instance, &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug messenger");
    }
}

void VulkanApp::initVulkanLoader()
{
    glfwInitVulkanLoader(vkGetInstanceProcAddr);
}

void VulkanApp::createWindow()
{   
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, GLFW_ANGLE_PLATFORM_TYPE_VULKAN);

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw library");
    }

    // This flag prevents the window from showing, but is necessary because otherwise the openGL context has ownership
    // over presentation on the window and Vulkan surface creation fails
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    window = glfwCreateWindow(1080, 720, "Vulkan App", NULL, NULL);

    glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);

}

void VulkanApp::createInstance()
{
    VkApplicationInfo appCreateInfo{};
    appCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appCreateInfo.pApplicationName = "Vulkan App";
    appCreateInfo.applicationVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.pEngineName = "No Engine";
    appCreateInfo.engineVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.apiVersion = VK_API_VERSION_1_2;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    

    // Retrieve the vulkan extensions required by glfw
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions = {};
    for (int index = 0 ; index < glfwExtensionCount ; index++) extensions.push_back(glfwExtensions[index]);

    if (enableValidationLayers)
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appCreateInfo;
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();
    instanceCreateInfo.enabledExtensionCount = extensions.size();
    instanceCreateInfo.pNext = NULL;
    if (enableValidationLayers) {
        instanceCreateInfo.enabledLayerCount = validationLayers.size();
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    }
    else {
        instanceCreateInfo.enabledLayerCount = 0;
        instanceCreateInfo.ppEnabledLayerNames = NULL;
    }

    // I dont know what allocation callbacks is (nullptr attribute), I'd imagine functions called
    // upon allocation of the resources
    VkResult res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
    if (res!= VK_SUCCESS) {
        throw std::runtime_error("Failed to create vulkan instance");
    }

}

void VulkanApp::selectPhysicalDevice()
{
    uint32_t physicalDeviceCount;

    // Retrieve the physical devices (each single complete implementation of vulkan)
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());

    // For the purposes of this app, I will just select my device manually (as I only have one)
    physicalDevice = physicalDevices[0];

}

void VulkanApp::setQueueIndices()
{
    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, familyProperties.data());

    int index = 0;
    for (VkQueueFamilyProperties family : familyProperties) {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueIndices.graphics = index;
        else if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
            queueIndices.compute = index;
        index++;
    }
}

void VulkanApp::createLogicalDeviceAndQueues()
{
    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo deviceGraphicsQueueCreateInfo{};
    deviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceGraphicsQueueCreateInfo.queueCount = 1;
    deviceGraphicsQueueCreateInfo.queueFamilyIndex = queueIndices.graphics;
    deviceGraphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceGraphicsQueueCreateInfo.pNext = NULL;

    VkDeviceQueueCreateInfo deviceComputeQueueCreateInfo{};
    deviceComputeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceComputeQueueCreateInfo.queueCount = 1;
    deviceComputeQueueCreateInfo.queueFamilyIndex = queueIndices.compute;
    deviceComputeQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceComputeQueueCreateInfo.pNext = NULL;

    VkDeviceQueueCreateInfo queueCreateInfos[] = {deviceGraphicsQueueCreateInfo, deviceComputeQueueCreateInfo};

    const char swapchainExtension[] = {"VK_KHR_swapchain"};
    const char* extensions[] = {swapchainExtension};

    VkDeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceCreateInfo.enabledExtensionCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = extensions;
    deviceCreateInfo.queueCreateInfoCount = 2;
    deviceCreateInfo.pQueueCreateInfos = queueCreateInfos;
    deviceCreateInfo.pNext = NULL;
    deviceCreateInfo.enabledLayerCount = 0;
    deviceCreateInfo.ppEnabledLayerNames = NULL;
    deviceCreateInfo.pEnabledFeatures = NULL; 
    deviceCreateInfo.flags = 0;

    vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);

    vkGetDeviceQueue(logicalDevice, queueIndices.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice, queueIndices.graphics, 0, &presentQueue);
    vkGetDeviceQueue(logicalDevice, queueIndices.compute, 0U, &computeQueue);
}

void VulkanApp::createWindowSurface()
{
    THROW(glfwCreateWindowSurface(instance, window, NULL, &surface), "Failed to create window surface");
}

void VulkanApp::createSwapchain()
{
    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilities);

    std::vector<VkSurfaceFormatKHR> supportedFormats = {};
    uint32_t counts;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &counts, NULL);
    supportedFormats.resize(counts);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &counts, supportedFormats.data());

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = surface;
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainCreateInfo.clipped = VK_FALSE;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainCreateInfo.imageExtent.height = windowHeight;
    swapchainCreateInfo.imageExtent.width = windowWidth;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.pNext = NULL;

    THROW(vkCreateSwapchainKHR(logicalDevice, &swapchainCreateInfo, NULL, &swapchain), "Failed to create swapchain");

    uint32_t swapchainImageCount = 0;
    // Store all swapchain images
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, NULL);
    swapchainImages.resize(swapchainImageCount);
    vkGetSwapchainImagesKHR(logicalDevice, swapchain, &swapchainImageCount, swapchainImages.data());
    maxFramesInFlight = swapchainImageCount;
}

// It is sometimes necessary to destroy the swapchain, this can occur if the window is resized 
void VulkanApp::recreateSwapchain()
{   
    vkDeviceWaitIdle(logicalDevice);
    cleanupSwapchain();
    createSwapchain();
    createImageViews();
    createFramebuffers();
}

void VulkanApp::cleanupSwapchain()
{
    /**
     * Clean up objects associated with this swapchain and its images:
     * 1. Framebuffers
     * 2. Image views
     * 3. Swapchain itself (which destroys its own images)
     */

    for (VkFramebuffer framebuffer : swapchainFramebuffers) vkDestroyFramebuffer(logicalDevice, framebuffer, NULL);
    for (VkImageView view : swapchainImageViews) vkDestroyImageView(logicalDevice, view, NULL);
    vkDestroySwapchainKHR(logicalDevice, swapchain, nullptr);
}

void VulkanApp::createDepthStencilTexAndView()
{
    depthStencilImage = resourceManager.createImageAll(windowWidth, windowHeight, AppImageTemplate::DEPTH_STENCIL);
}

// To use any VkImage (including those created by the swapchain) we must create a view (VkImageView)
// Which describes how to access the image and which part to access
void VulkanApp::createImageViews()
{
    swapchainImageViews.resize(swapchainImages.size());

    for (uint32_t index = 0 ; index < swapchainImages.size() ; index++) {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = swapchainImages[index];
        imageViewCreateInfo.format = VK_FORMAT_B8G8R8A8_SRGB;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;
        VkResult res = vkCreateImageView(logicalDevice, &imageViewCreateInfo, NULL, &(swapchainImageViews[index]));
    }
}

void VulkanApp::createGraphicsPipeline()
{
    // These are pipeline states that can be changed without recreating the pipeline at draw time
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    // Configure vertex buffer binding

    VkVertexInputBindingDescription vertBindDesc{};
    vertBindDesc.binding = 0;
    vertBindDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    vertBindDesc.stride = sizeof(Vertex);

    VkVertexInputAttributeDescription attributeDesc[3] = {
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

        // Texcoord
        {
            2U, // Location, the shader input location (layout(location = 2) in vec2 texCoord)
            0U, // Binding, the binding number of the vertex buffer from which this data is coming,
            VK_FORMAT_R32G32_SFLOAT, // Format, we use a Float2 for the texture coordinate,
            24U // Offset, we use 24 bytes since position & normal take up 6 x 4-byte values
        }
    };

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &vertBindDesc; 
    vertexInputInfo.vertexAttributeDescriptionCount = 3;
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
    viewport.width = (float) windowWidth;
    viewport.height = (float) windowHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Configure the scissor (can be used to discard rasterizer pixels)
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = windowWidth;
    scissor.extent.height = windowHeight;

    // Since we are dynamically specifying the viewport and scissor struct, theres no need to specify them in the viewport
    // state. We will set these values up later at draw time.
    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

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

    // Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 1; // Optional
    pipelineLayoutInfo.pSetLayouts = &pipelineDescriptorSetLayout; // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional
    if (vkCreatePipelineLayout(logicalDevice, &pipelineLayoutInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create pipeline layout");
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertexShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragmentShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

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
    colorSubpassPipelineInfo.stageCount = 2;
    colorSubpassPipelineInfo.pStages = shaderStages;
    colorSubpassPipelineInfo.pVertexInputState = &vertexInputInfo;
    colorSubpassPipelineInfo.pInputAssemblyState = &inputAssembly;
    colorSubpassPipelineInfo.pViewportState = &viewportState;
    colorSubpassPipelineInfo.pRasterizationState = &rasterizer;
    colorSubpassPipelineInfo.pMultisampleState = &multisampling;
    colorSubpassPipelineInfo.pDepthStencilState = &dsStateCreateInfo; // Optional
    colorSubpassPipelineInfo.pColorBlendState = &colorBlending;
    colorSubpassPipelineInfo.pDynamicState = &dynamicState;
    colorSubpassPipelineInfo.layout = pipelineLayout;
    colorSubpassPipelineInfo.renderPass = renderPass;
    colorSubpassPipelineInfo.subpass = 0;
    colorSubpassPipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    colorSubpassPipelineInfo.basePipelineIndex = -1; // Optional
    
    if (vkCreateGraphicsPipelines(logicalDevice, VK_NULL_HANDLE, 1, &colorSubpassPipelineInfo, NULL, &colorGraphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create graphics pipeline");
    }

}

void VulkanApp::createRenderPass()
{
    /**
     * Here we define our renderpass (and subpasses).
     * 
     * Attachments: There are 4 types of attachments that
     * a subpass can have:
     * - Color Attachments -> Written into by the fragment shader
     * - Depth and Stencil Attachments -> 
     * - Resolve Attachments -> Used for MSAA
     * - Input Attachment -> Can be read as input by subsequent subpasses within the same pass
     * 
     * Subpass: Defines a subpass and its attachments
     *
     * Subpass Dependency: 
     */

    // Render out depth first, then color as a test
    enum class Subpass {
        COLOR = 0
    };

    enum class Attachment {
        COLOR = 0,
        DEPTH = 1
    };

    // ---------------------- Define the depth stencil subpass ---------------------
    /**
     * Attachment descriptions are used for the whole render pass
     * format: This attachment is the depth/stencil attachment, we use an appropriate format
     * samples: Conduct 1 sample per pixel. This is used for MSAA, but we are not using MSAA
     * loadOp: Clear the existing contents at the start of the render pass.
     * storeOp: Value doesn't matter after the renderpass, it can be cleared. Depth info is not used once the pass is complete.
     * stencilLoadOp: Clear the stencil data to default values at the start of the render pass.
     * stencilStoreOp: Value doesn't matter after the render pass as it is only used during the render pass.
     * initialLayout: Undefined layout at first, since we will be clearing the data as we enter the render pass.
     * finalLayout: Transitions to a read only depth stencil attachment layout following the depth subpass.
     */
    VkAttachmentDescription depthStencilAttachment{};
    depthStencilAttachment.format = VK_FORMAT_D32_SFLOAT_S8_UINT;
    depthStencilAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthStencilAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthStencilAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    /**
     * Attachment references are per subpass, Attachment descriptions apply to the entirety of the renderpass
     * attachment: 0, as this is the 0th and only attachment during the depth pre-pass
     * layout: The layout that is transitioned to (writeable depth stencil) for the duration of the subpass
     */

    VkAttachmentReference colorAttachmentColorPassRef{};
    colorAttachmentColorPassRef.attachment = 0;
    colorAttachmentColorPassRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference dsAttachmentColorPassRef{};
    dsAttachmentColorPassRef.attachment = 1;
    dsAttachmentColorPassRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Define the color subpass
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = VK_FORMAT_B8G8R8A8_SRGB;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkSubpassDescription colorSubpass{};
    colorSubpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    colorSubpass.colorAttachmentCount = 1;
    colorSubpass.pColorAttachments = &colorAttachmentColorPassRef;
    colorSubpass.pDepthStencilAttachment = &dsAttachmentColorPassRef;

    VkSubpassDependency subpassDep{};
    subpassDep.dstSubpass = 0U;
    subpassDep.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDep.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDep.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    subpassDep.srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

    /**
     * Describes the entirety of the renderpass (all subpasses, their dependencies
     * and their attachments)
     */
    VkAttachmentDescription attachments[] = { colorAttachment, depthStencilAttachment };
    VkSubpassDependency dependencies[] = { subpassDep };
    VkSubpassDescription subpasses[] = { colorSubpass };
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = NULL;
    renderPassInfo.attachmentCount = 2;
    renderPassInfo.pAttachments = attachments;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = subpasses;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = dependencies;

    if (vkCreateRenderPass(logicalDevice, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanApp::createShaderModules()
{   
    std::vector<char> vertexShaderFile = readFile("/home/alessandro/Projects/VulkanApp1/shaders/build/vert.spv");
    std::vector<char> fragmentShaderFile = readFile("/home/alessandro/Projects/VulkanApp1/shaders/build/frag.spv");
    VkShaderModuleCreateInfo vertShaderCreateInfo{};
    vertShaderCreateInfo.codeSize = vertexShaderFile.size();
    vertShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    vertShaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(vertexShaderFile.data());
    if (vkCreateShaderModule(logicalDevice, &vertShaderCreateInfo, NULL, &(vertexShaderModule)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex shader");
    }

    VkShaderModuleCreateInfo fragShaderCreateInfo{};
    fragShaderCreateInfo.codeSize = fragmentShaderFile.size();
    fragShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    fragShaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(fragmentShaderFile.data());
    if (vkCreateShaderModule(logicalDevice, &fragShaderCreateInfo, NULL, &(fragmentShaderModule)) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create fragment shader");
    }
}

void VulkanApp::createFramebuffers()
{
    swapchainFramebuffers.resize(swapchainImageViews.size());
    for (uint32_t index = 0u ; index < swapchainImageViews.size() ; index++) {
        VkImageView attachments[] = {
            swapchainImageViews[index],
            depthStencilImage.view
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 2;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = windowWidth;
        framebufferInfo.height = windowHeight;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logicalDevice, &framebufferInfo, NULL, &(swapchainFramebuffers[index])) != VK_SUCCESS) {
            throw std::runtime_error("Failed to create framebuffers");
        }
    }
}

void VulkanApp::createCommandPool()
{
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueIndices.graphics;

    if (vkCreateCommandPool(logicalDevice, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create command pool");
    }
}

void VulkanApp::createCommandBuffer()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    if (vkAllocateCommandBuffers(logicalDevice, &allocInfo, &commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers");
    }
}

void VulkanApp::createSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    // Start the fence off as signalled so that the first wait is bypassed
    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    
    if (vkCreateSemaphore(logicalDevice, &semaphoreInfo, NULL, &imageAvailableSemaphore) != VK_SUCCESS ||
        vkCreateSemaphore(logicalDevice, &semaphoreInfo, NULL, &renderingFinishedSemaphore) != VK_SUCCESS ||
        vkCreateFence(logicalDevice, &fenceInfo, NULL, &inFlightFence )!= VK_SUCCESS) {
            throw std::runtime_error("Failed to create sync objects");
    }

}

void VulkanApp::createVertexBuffer()
{
    stagingVertexBuffer = resourceManager.createBufferAll(AppBufferTemplate::VERTEX_BUFFER_STAGING, sizeof(Vertex) * supportedVertexCount);
    deviceVertexBuffer = resourceManager.createBufferAll(AppBufferTemplate::VERTEX_BUFFER_DEVICE, sizeof(Vertex) * supportedVertexCount);
}

void VulkanApp::createVSUniformBuffers()
{
    uniformBuffersVS.resize(maxFramesInFlight);
    for (uint32_t frame = 0u ; frame < maxFramesInFlight ; frame++) {
        uniformBuffersVS[frame] = resourceManager.createBufferAll(AppBufferTemplate::UNIFORM_BUFFER, sizeof(VSUniformBuffer));
    }
}

// Provide the details for all descriptor bindings we will use (just one for this project)
void VulkanApp::createDescriptorSetLayout()
{
    pipelineDescriptorSetLayout = resourceManager.createDescriptorSetLayout({
        AppDescriptorItemTemplate::VS_UNIFORM_BUFFER,
        AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER,
        AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER
    });
}

void VulkanApp::createDescriptorPool()
{
    descriptorPool = resourceManager.createDescriptorPool(maxFramesInFlight, {
        {AppDescriptorItemTemplate::VS_UNIFORM_BUFFER, maxFramesInFlight},
        {AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, 2}
    });
}

void VulkanApp::createDescriptorSets()
{
    for (uint32_t frame = 0u; frame < maxFramesInFlight ; frame++) {
        uboDescriptorSets.push_back(resourceManager.allocateDescriptorSet(pipelineDescriptorSetLayout, descriptorPool));
    }
}

void VulkanApp::updateDescriptorSets()
{
    for (uint32_t frame = 0u ; frame < maxFramesInFlight ; frame++) {
        resourceManager.updateBufferDescriptor(uboDescriptorSets[frame], uniformBuffersVS[frame].buffer, sizeof(VSUniformBuffer), 0U, AppDescriptorItemTemplate::VS_UNIFORM_BUFFER);
        resourceManager.updateImageDescriptor(albedo, uboDescriptorSets[frame], 1U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);
        resourceManager.updateImageDescriptor(normal, uboDescriptorSets[frame], 2U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);
    }
}

void VulkanApp::createIndexBuffer()
{
    stagingIndexBuffer = resourceManager.createBufferAll(AppBufferTemplate::INDEX_BUFFER_STAGING, sizeof(uint32_t) * supportedIndexCount);
    deviceIndexBuffer = resourceManager.createBufferAll(AppBufferTemplate::INDEX_BUFFER_DEVICE, sizeof(uint32_t) * supportedIndexCount);
}

void VulkanApp::mapUBOs()
{
    mappedUBOs.resize(maxFramesInFlight);
    for (uint32_t frame = 0u ; frame < maxFramesInFlight ; frame++) {
        vkMapMemory(logicalDevice, uniformBuffersVS[frame].memory, 0u, sizeof(VSUniformBuffer), 0, &(mappedUBOs[frame]));
    }
}

void VulkanApp::updateUBO(uint32_t frame)
{
    VSUniformBuffer ubo;
    ubo.viewMatrix = appCamera.getViewMatrix();
    ubo.projMatrix = appCamera.getProjMatrix();
    memcpy(mappedUBOs[frame], &ubo, sizeof(VSUniformBuffer));
}

void VulkanApp::tickTimer()
{
    deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(highResClock.now() - lastRenderTime);
    lastRenderTime = highResClock.now();
}

void VulkanApp::createSampler()
{
    /**
     * magFilter: The magnification filter to apply when the texel (texture element) size is smaller than
     * the pixel size
     */
    VkSamplerCreateInfo samplerCreateInfo{};
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.pNext = nullptr;
    samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
    samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerCreateInfo.mipLodBias = 0.f;
    samplerCreateInfo.anisotropyEnable = VK_FALSE;
    samplerCreateInfo.maxAnisotropy = 16.0f;
    samplerCreateInfo.compareEnable = VK_TRUE;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerCreateInfo.minLod = 0.f;
    samplerCreateInfo.maxLod = 1.f;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    
    THROW(vkCreateSampler(logicalDevice, &samplerCreateInfo, nullptr, &sampler), "Failed to create sampler");
}

void VulkanApp::updateVIBuffers()
{
    resourceManager.copyDataToStagingBuffer(stagingVertexBuffer, meshManager.getVertexData(), meshManager.getVertexDataSize());
    resourceManager.pushStagingBuffer(stagingVertexBuffer, deviceVertexBuffer);

    resourceManager.copyDataToStagingBuffer(stagingIndexBuffer, meshManager.getIndexData(), meshManager.getIndexDataSize());
    resourceManager.pushStagingBuffer(stagingIndexBuffer, deviceIndexBuffer);
}

void VulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0; // Optional
    beginInfo.pInheritanceInfo = nullptr; // Optional

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("Failed to begin recording command buffer");
    }

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent.width = windowWidth;
    renderPassInfo.renderArea.extent.height = windowHeight;

    VkClearValue clearValues[2]{};
    clearValues[0].color = {{0.f, 0.f, 0.f, 1.f}};
    clearValues[1].depthStencil = {1.f, 0u};
    renderPassInfo.clearValueCount = 2;
    renderPassInfo.pClearValues = clearValues;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, colorGraphicsPipeline);

    VkDeviceSize offsets[] = {0UL};
    vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, &deviceVertexBuffer.buffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, deviceIndexBuffer.buffer, 0u, VkIndexType::VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0u, 1u, &(uboDescriptorSets[imageIndex]),
    0, nullptr);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(windowWidth);
    viewport.height = static_cast<float>(windowHeight);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = windowWidth;
    scissor.extent.height = windowHeight;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    uint32_t meshCount = meshManager.getMeshCount();
    for (uint32_t meshIndex = 0U ; meshIndex < meshCount ; meshIndex++) {
        MeshManager::MeshInsertion insertion = meshManager.getMeshInsertionAtIndex(meshIndex);
        vkCmdDrawIndexed(commandBuffer, insertion.indexCount, 1u, insertion.indexOffset, insertion.vertexOffset, 0u);
    }

    vkCmdEndRenderPass(commandBuffer);

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to record command buffer");
    }
}

void VulkanApp::glfwCursorPositionCallback(GLFWwindow *window, double xpos, double ypos)
{
    appHandle->appCursorPositionCallback(xpos, ypos);
}

void VulkanApp::glfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    appHandle->appKeyCallback(key, scancode, action, mods);
}

void VulkanApp::appCursorPositionCallback(double xpos, double ypos)
{
    double deltaX = xpos - xposOld;
    double deltaY = ypos - yposOld;


    // Get the real world camera angle represented by each pixel (as the vertical angle divided by the number of pixels in screen space).
    // The horizontal angle per pixel is equivalent, provided that the HFOV is VFOV * Aspect Ratio
    const float radiansPerPixel = appCamera.getVFOV() / windowHeight;

    // Query mouse button state
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS) {
        appCamera.lookUp(deltaY * 1.f * radiansPerPixel);
        appCamera.lookRight(deltaX * -1.f * radiansPerPixel);
    }

    yposOld = ypos;
    xposOld = xpos;
}

void VulkanApp::appKeyCallback(int key, int scancode, int action, int mods)
{   
    if (action == GLFW_PRESS || action == GLFW_RELEASE){
        if (key == GLFW_KEY_W) {
            wDown = action == GLFW_PRESS;
        }
        else if (key == GLFW_KEY_S) {
            sDown = action == GLFW_PRESS;
        }
        else if (key == GLFW_KEY_D) {
            dDown = action == GLFW_PRESS;
        }
        else if (key == GLFW_KEY_A) {
            aDown = action == GLFW_PRESS;
        }
        else if (key == GLFW_KEY_E) {
            eDown = action == GLFW_PRESS;
        }
        else if (key == GLFW_KEY_Q) {
            qDown = action == GLFW_PRESS;
        }
    }
    
}

void VulkanApp::processKeyActions()
{
    const float movementSpeedPerSecond = 10.0f;
    float dist = movementSpeedPerSecond * deltaTime.count();
    if (wDown) appCamera.moveForward(dist);
    if (sDown) appCamera.moveForward(dist * -1.f);
    if (dDown) appCamera.moveRight(dist);
    if (aDown) appCamera.moveRight(dist * -1.f);
    if (eDown) appCamera.moveUp(dist);
    if (qDown) appCamera.moveUp(dist * -1.f);
}

void VulkanApp::init()
{
    // keep track of the handle to call this app's methods from glfw static callback functions
    appHandle = this;
    meshManager.init(this);
    resourceManager.init(this);
    initVulkanLoader();
    createWindow();
    createInstance();
    if (enableValidationLayers){
        setupDebugMessenger();
    }
    selectPhysicalDevice();
    setQueueIndices();
    createLogicalDeviceAndQueues();
    createCommandPool();
    createCommandBuffer();
    createDepthStencilTexAndView();

    createAndLoadVulkanImage("../images/muddy_base.ppm", this, albedo);
    createAndLoadVulkanImage("../images/muddy_normal.ppm", this, normal);

    createWindowSurface();
    createVertexBuffer();
    createIndexBuffer();
    createSwapchain();
    createVSUniformBuffers();
    createSampler();
    createDescriptorSetLayout();
    createDescriptorPool();
    createDescriptorSets();
    updateDescriptorSets();
    mapUBOs();
    createImageViews();
    createShaderModules();
    createRenderPass();
    createFramebuffers();
    createGraphicsPipeline();
    createSyncObjects();
    computeShaderSetup(&resourceManager);


    lastRenderTime = highResClock.now();
}

void VulkanApp::drawFrame(float deltaTime)
{
    /**
     * Drawing in vulkan usually consists of the following events
     * 1. Wait for the previous frame to finish rendering
     * 2. Aquire an image from the swap chain, we will be rendering to this image
     * 3. Record a command buffer which draws the scene onto our image
     * 4. Submit the recorded command buffer which draws the scene onto our image
     * 5. Present the new swap chain image
     * 
     * Many of Vulkan's API calls which execute work on the GPU happen asynchronously, meaning we need to
     * enforce order. 
     */

    // Step 1. Wait for the previous frame to render
    vkWaitForFences(logicalDevice, 1, &inFlightFence, VK_TRUE, UINT32_MAX);

    //computeShaderWriteCommandBuffer(&resourceManager);

    // Step 2. Aquire an image from the swap chain (well an index to it actually)
    uint32_t freeImageIndex;
    VkResult acquireImageResult = vkAcquireNextImageKHR(logicalDevice, swapchain, UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &freeImageIndex);
    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireImageResult == VK_SUBOPTIMAL_KHR) {
        recreateSwapchain();
        return;
    }
    else if (acquireImageResult != VK_SUCCESS) {
        throw std::runtime_error("Failed to acquire a swapchain image");
    }

    // Return fence to unsignaled state (only if we know we successfully retrieved an image, otherwise we may end up in a deadlock)
    vkResetFences(logicalDevice, 1, &inFlightFence);

    // Step 3. Record a command buffer to draw the scene onto the image
    vkResetCommandBuffer(commandBuffer, 0U);
    recordCommandBuffer(commandBuffer, freeImageIndex);

    // Step 4. Submit the command buffer to be run
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    VkSemaphore signalSemaphores[] = {renderingFinishedSemaphore};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;
    submitInfo.pNext = nullptr;
    updateUBO(freeImageIndex);

    VkResult res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Step 5. Present the new swap chain image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &freeImageIndex;
    presentInfo.pResults = nullptr;

    vkQueuePresentKHR(presentQueue, &presentInfo);
}

void VulkanApp::renderLoop()
{
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        processKeyActions();
        tickTimer();
        drawFrame(deltaTime.count());
    }

    // Wait for all operations to clear up before exiting so that all objects can be destroyed
    vkDeviceWaitIdle(logicalDevice);
}

void VulkanApp::cleanup()
{
    resourceManager.destroy();
    computeShaderCleanup(&resourceManager);
    vkDestroySampler(logicalDevice, sampler, nullptr);
    vkDestroyShaderModule(logicalDevice, vertexShaderModule, NULL);
    vkDestroyShaderModule(logicalDevice, fragmentShaderModule, NULL);
    vkDestroySemaphore(logicalDevice, imageAvailableSemaphore, NULL);
    vkDestroySemaphore(logicalDevice, renderingFinishedSemaphore, NULL);
    vkDestroyFence(logicalDevice, inFlightFence, NULL);
    vkDestroyCommandPool(logicalDevice, commandPool, NULL);
    vkDestroyPipeline(logicalDevice, colorGraphicsPipeline, NULL);
    vkDestroyPipelineLayout(logicalDevice, pipelineLayout, NULL);
    vkDestroyRenderPass(logicalDevice, renderPass, NULL);
    cleanupSwapchain();
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(logicalDevice, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

#include "vulkan-app.h"
#include <iostream>
#include "GLFW/glfw3.h"
#include <mesh.h>
#include <attach.h>
#include <utilities.h>

VulkanApp* appHandle;

bool VulkanApp::isPhysicalDeviceSuitable(VkPhysicalDevice device)
{
    return true;
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

    THROW(glfwCreateWindowSurface(instance, window, NULL, &surface), "Failed to create window surface");

    commandPool = resourceManager.createCommandPool(queueIndices.graphics);
    commandBuffer = resourceManager.allocateCommandBuffer(commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

    albedo = createAndLoadVulkanImage("../images/muddy_base.ppm", this);
    normal = createAndLoadVulkanImage("../images/muddy_normal.ppm", this);

    // Create the vertex and index staging buffers
    stagingVertexBuffer = resourceManager.createBufferAll(AppBufferTemplate::VERTEX_BUFFER_STAGING, sizeof(Vertex) * supportedVertexCount);
    deviceVertexBuffer = resourceManager.createBufferAll(AppBufferTemplate::VERTEX_BUFFER_DEVICE, sizeof(Vertex) * supportedVertexCount);
    stagingIndexBuffer = resourceManager.createBufferAll(AppBufferTemplate::INDEX_BUFFER_STAGING, sizeof(uint32_t) * supportedIndexCount);
    deviceIndexBuffer = resourceManager.createBufferAll(AppBufferTemplate::INDEX_BUFFER_DEVICE, sizeof(uint32_t) * supportedIndexCount);

    swapchain = resourceManager.createSwapchain();
    depthStencilImage = resourceManager.createImageAll(windowWidth, windowHeight, AppImageTemplate::DEPTH_STENCIL);

    descriptorPool = resourceManager.createDescriptorPool(maxFramesInFlight, {
        {AppDescriptorItemTemplate::VS_UNIFORM_BUFFER, maxFramesInFlight},
        {AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, 2}
    });

    pipelineDescriptorSetLayout = resourceManager.createDescriptorSetLayout({
        AppDescriptorItemTemplate::VS_UNIFORM_BUFFER,
        AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER,
        AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER
    });

    // Create the application render pass
    renderPass = resourceManager.createRenderPass(
        // Attachments
        { 
            {AppAttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT},
            {AppAttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT}
        },
        // Subpasses
        {
            // subpass 1
            AppSubpass {
                {
                    AppSubpassAttachmentRef{0U, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL},
                    AppSubpassAttachmentRef{1U, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL}
                }
            }
        },
        {
            // subpass dependency
            {VK_SUBPASS_EXTERNAL, 0U, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0U, VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, 0U}
        }
    );

    sampler = resourceManager.createSampler(AppSamplerTemplate::DEFAULT);

    for (uint32_t frame = 0u; frame < maxFramesInFlight ; frame++) {
        // Create a uniform buffer for all frames in flight
        uniformBuffersVS.push_back(resourceManager.createBufferAll(AppBufferTemplate::UNIFORM_BUFFER, sizeof(VSUniformBuffer)));

        // Allocate the descriptor sets for each frame
        descriptorSetsPerFrame.push_back(resourceManager.allocateDescriptorSet(pipelineDescriptorSetLayout, descriptorPool));

        // Update all descriptors associated with the descriptor set for this frame
        resourceManager.updateBufferDescriptor(descriptorSetsPerFrame[frame], uniformBuffersVS[frame].buffer.get(), sizeof(VSUniformBuffer), 0U, AppDescriptorItemTemplate::VS_UNIFORM_BUFFER);
        resourceManager.updateImageDescriptor(albedo.imageView, descriptorSetsPerFrame[frame], 1U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);
        resourceManager.updateImageDescriptor(normal.imageView, descriptorSetsPerFrame[frame], 2U, AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER, sampler);

        // Map the memory for the Uniform Buffer Objects
        mappedUBOs.push_back({});
        vkMapMemory(logicalDevice, uniformBuffersVS[frame].deviceMemory.get(), 0u, sizeof(VSUniformBuffer), 0, &(mappedUBOs[frame]));

        // Create a framebuffer for the resources of this frame
        swapchain.framebuffers.push_back(resourceManager.createFramebuffer(renderPass, {
            swapchain.swapchainImageViews[frame].get(),
            depthStencilImage.imageView.get()
        }));
    }

    // Create the shader modules that will be used
    vertexShaderModule = resourceManager.createShaderModule("../shaders/build/vert.spv", VK_SHADER_STAGE_VERTEX_BIT);
    fragmentShaderModule = resourceManager.createShaderModule("../shaders/build/frag.spv", VK_SHADER_STAGE_FRAGMENT_BIT);

    // Create the pipeline layout and pipeline
    pipelineLayout = resourceManager.createPipelineLayout({pipelineDescriptorSetLayout});
    colorGraphicsPipeline = resourceManager.createGraphicsPipeline(
        {vertexShaderModule, fragmentShaderModule},
        pipelineLayout,
        renderPass
    );

    // Create some sync primitives that we'll use during rendering
    inFlightFence = resourceManager.createFence(true);
    renderingFinishedSemaphore = resourceManager.createSemaphore();
    imageAvailableSemaphore = resourceManager.createSemaphore();

    lastRenderTime = highResClock.now();
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
    renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex].get();
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
    VkBuffer vertexBuffers[] = {deviceVertexBuffer.buffer.get()};
    vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, vertexBuffers, offsets);
    vkCmdBindIndexBuffer(commandBuffer, deviceIndexBuffer.buffer.get(), 0u, VkIndexType::VK_INDEX_TYPE_UINT32);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0u, 1u, &(descriptorSetsPerFrame[imageIndex]),
    0, nullptr);

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
    VkResult acquireImageResult = vkAcquireNextImageKHR(logicalDevice, swapchain.get(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &freeImageIndex);
    if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireImageResult == VK_SUBOPTIMAL_KHR) {
        resourceManager.destroySwapchain(swapchain);

        // Recreate swapchain
        swapchain = resourceManager.createSwapchain();

        // Recreate all framebuffers
        for (uint32_t frame = 0U ; frame < maxFramesInFlight ; frame)
            swapchain.framebuffers.push_back(resourceManager.createFramebuffer(renderPass, {swapchain.swapchainImageViews[frame].get(), depthStencilImage.imageView.get()}));
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

    VSUniformBuffer ubo;
    ubo.viewMatrix = appCamera.getViewMatrix();
    ubo.projMatrix = appCamera.getProjMatrix();
    memcpy(mappedUBOs[freeImageIndex], &ubo, sizeof(VSUniformBuffer));

    VkResult res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit draw command buffer");
    }

    // Step 5. Present the new swap chain image
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain.get()};
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

void VulkanApp::tickTimer()
{
    deltaTime = std::chrono::duration_cast<std::chrono::duration<double>>(highResClock.now() - lastRenderTime);
    lastRenderTime = highResClock.now();
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


void VulkanApp::cleanup()
{
    resourceManager.destroy();
    //computeShaderCleanup(&resourceManager);
    vkDestroySurfaceKHR(instance, surface, NULL);
    vkDestroyDevice(logicalDevice, nullptr);
    if (enableValidationLayers)
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
    vkDestroyInstance(instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

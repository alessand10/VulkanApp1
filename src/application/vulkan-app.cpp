#include "vulkan-app.h"
#include <iostream>
#include "GLFW/glfw3.h"
#include <mesh.h>

VulkanApp* appHandle;

void VulkanApp::init()
{
    // Initialize the vulkan loader
    glfwInitVulkanLoader(vkGetInstanceProcAddr);

    // keep track of the handle to call this app's methods from glfw static callback functions
    appHandle = this;
    createWindow();

    instance.init(this, "Vulkan App", true);
    
    setupDebugMessenger();

    enumeratePhysicalDevice();

    logicalDevice.init(this, physicalDevice);
    vkGetDeviceQueue(logicalDevice.get(), queueFamilyIndices.graphics, 0, &graphicsQueue);
    vkGetDeviceQueue(logicalDevice.get(), queueFamilyIndices.graphics, 0, &presentQueue);
    vkGetDeviceQueue(logicalDevice.get(), queueFamilyIndices.compute, 0U, &computeQueue);

    surface.init(this, window);

    lastRenderTime = highResClock.now();

}

void VulkanApp::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    // VkCommandBufferBeginInfo beginInfo{};
    // beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    // beginInfo.flags = 0; // Optional
    // beginInfo.pInheritanceInfo = nullptr; // Optional

    // if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to begin recording command buffer");
    // }

    // VkRenderPassBeginInfo renderPassInfo{};
    // renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    // renderPassInfo.renderPass = renderPass;
    // renderPassInfo.framebuffer = swapchain.framebuffers[imageIndex].get();
    // renderPassInfo.renderArea.offset = {0, 0};
    // renderPassInfo.renderArea.extent.width = windowWidth;
    // renderPassInfo.renderArea.extent.height = windowHeight;

    // VkClearValue clearValues[2]{};
    // clearValues[0].color = {{0.f, 0.f, 0.f, 1.f}};
    // clearValues[1].depthStencil = {1.f, 0u};
    // renderPassInfo.clearValueCount = 2;
    // renderPassInfo.pClearValues = clearValues;

    // vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // // Push the texture constants
    // FragmentPushConst pc {1U};
    // vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0U, sizeof(FragmentPushConst), &pc);

    // VkDeviceSize offsets[] = {0UL};
    // VkBuffer vertexBuffers[] = {deviceVertexBuffer.buffer.get()};
    // vkCmdBindVertexBuffers(commandBuffer, 0u, 1u, vertexBuffers, offsets);
    // vkCmdBindIndexBuffer(commandBuffer, deviceIndexBuffer.buffer.get(), 0u, VkIndexType::VK_INDEX_TYPE_UINT32);

    // vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0u, 1u, &(descriptorSetsPerFrame[imageIndex]),
    // 0, nullptr);

    // uint32_t meshCount = meshManager.getMeshCount();
    // for (uint32_t meshIndex = 0U ; meshIndex < meshCount ; meshIndex++) {
    //     GeometryManager::MeshInsertion insertion = meshManager.getMeshInsertionAtIndex(meshIndex);
    //     vkCmdDrawIndexed(commandBuffer, insertion.indexCount, 1u, insertion.indexOffset, insertion.vertexOffset, 0u);
    // }

    // vkCmdEndRenderPass(commandBuffer);

    // if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to record command buffer");
    // }
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
    // vkWaitForFences(logicalDevice.get(), 1, &inFlightFence, VK_TRUE, UINT32_MAX);

    // //computeShaderWriteCommandBuffer(&resourceManager);

    // // Step 2. Aquire an image from the swap chain (well an index to it actually)
    // uint32_t freeImageIndex;
    // VkResult acquireImageResult = vkAcquireNextImageKHR(logicalDevice.get(), swapchain.get(), UINT64_MAX, imageAvailableSemaphore, VK_NULL_HANDLE, &freeImageIndex);
    // if (acquireImageResult == VK_ERROR_OUT_OF_DATE_KHR || acquireImageResult == VK_SUBOPTIMAL_KHR) {
    //     resourceManager.destroySwapchain(swapchain);

    //     // Recreate swapchain
    //     swapchain = resourceManager.createSwapchain();

    //     // Recreate all framebuffers
    //     for (uint32_t frame = 0U ; frame < maxFramesInFlight ; frame)
    //         swapchain.framebuffers.push_back(resourceManager.createFramebuffer(renderPass, {swapchain.swapchainImageViews[frame].get(), depthStencilImage.imageView.get()}));
    //     return;
    // }
    // else if (acquireImageResult != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to acquire a swapchain image");
    // }

    // // Return fence to unsignaled state (only if we know we successfully retrieved an image, otherwise we may end up in a deadlock)
    // vkResetFences(logicalDevice.get(), 1, &inFlightFence);

    // // Step 3. Record a command buffer to draw the scene onto the image
    // vkResetCommandBuffer(commandBuffer, 0U);
    // recordCommandBuffer(commandBuffer, freeImageIndex);

    // // Step 4. Submit the command buffer to be run
    // VkSubmitInfo submitInfo{};
    // submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    // VkSemaphore waitSemaphores[] = {imageAvailableSemaphore};
    // VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    // submitInfo.waitSemaphoreCount = 1;
    // submitInfo.pWaitSemaphores = waitSemaphores;
    // submitInfo.pWaitDstStageMask = waitStages;
    // submitInfo.commandBufferCount = 1;
    // submitInfo.pCommandBuffers = &commandBuffer;

    // VkSemaphore signalSemaphores[] = {renderingFinishedSemaphore};
    // submitInfo.signalSemaphoreCount = 1;
    // submitInfo.pSignalSemaphores = signalSemaphores;
    // submitInfo.pNext = nullptr;

    // VSUniformBuffer ubo;
    // ubo.viewMatrix = appCamera.getViewMatrix();
    // ubo.projMatrix = appCamera.getProjMatrix();
    // memcpy(mappedUBOs[freeImageIndex], &ubo, sizeof(VSUniformBuffer));

    // VkResult res = vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFence);
    // if (res != VK_SUCCESS) {
    //     throw std::runtime_error("Failed to submit draw command buffer");
    // }

    // // Step 5. Present the new swap chain image
    // VkPresentInfoKHR presentInfo{};
    // presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    // presentInfo.waitSemaphoreCount = 1;
    // presentInfo.pWaitSemaphores = signalSemaphores;

    // VkSwapchainKHR swapChains[] = {swapchain.get()};
    // presentInfo.swapchainCount = 1;
    // presentInfo.pSwapchains = swapChains;
    // presentInfo.pImageIndices = &freeImageIndex;
    // presentInfo.pResults = nullptr;

    // vkQueuePresentKHR(presentQueue, &presentInfo);
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
    vkDeviceWaitIdle(logicalDevice.get());
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

void VulkanApp::enumeratePhysicalDevice()
{
    uint32_t physicalDeviceCount;

    // Retrieve the physical device counts (each single complete implementation of vulkan)
    vkEnumeratePhysicalDevices(instance.get(), &physicalDeviceCount, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);

    // Retrieve the actual physical devices
    vkEnumeratePhysicalDevices(instance.get(), &physicalDeviceCount, physicalDevices.data());

    // For the purposes of this app, I will just select my device manually (as I only have one)
    physicalDevice = physicalDevices[0];

}

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

    if (CreateDebugUtilsMessengerEXT(instance.get(), &createInfo, NULL, &debugMessenger) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create debug messenger");
    }
}

void VulkanApp::createWindow()
{   
    #ifdef WINDOWS_PLATFORM
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WIN32);
    #elif defined(LINUX_PLATFORM)
        glfwInitHint(GLFW_ANGLE_PLATFORM_TYPE, GLFW_PLATFORM_X11);
    #endif

    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize glfw library");
    }

    // This flag prevents the window from showing, but is necessary because otherwise the openGL context has ownership
    // over presentation on the window and Vulkan surface creation fails
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);
    window = glfwCreateWindow(windowWidth, windowHeight, "Vulkan App", NULL, NULL);

    glfwSetCursorPosCallback(window, glfwCursorPositionCallback);
    glfwSetKeyCallback(window, glfwKeyCallback);

}

void VulkanApp::cleanup()
{
    DestroyDebugUtilsMessengerEXT(instance.get(), debugMessenger, nullptr);
    glfwDestroyWindow(window);
    resources.destroyAll(logicalDevice.get());
    glfwTerminate();
}

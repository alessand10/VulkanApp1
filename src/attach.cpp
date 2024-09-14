#include <vulkan/vulkan.hpp>
#include "resource-manager.h"
#include "attach.h"
#include "vulkan-app.h"
#include "utilities.h"
/**
 * The goal of this task is to create a Vulkan compute shader pipeline that takes the 
 * pixel position and time as inputs and writes to an image.
 * 
 * We need
 * - 1 Compute pipeline
 * - 1 UBO bound via descriptor
 * - 1 Output image bound via descriptor
 * - 1 Descriptor set to bind to the pipeline (each descriptor set is bound to an entire pipeline)
 */

struct UBO {
    float time = 0.0f;
};

VkBuffer computeUBO;
VkDeviceMemory computeUBOMemory;

AppImageBundle outputImage;

VkDescriptorSetLayout descriptorSetlayout;
VkDescriptorSet computePipelineDescriptorSet;
VkDescriptorPool computeShaderDescriptorPool;

VkShaderModule computeShaderModule;

VkPipelineLayout computePipelineLayout;
VkPipeline computePipeline;

VkCommandPool computeCommandPool;
VkCommandBuffer computeCommandBuffer;

VkFence computeShaderBusyFence;

uint32_t workGroupSizeX = 32U, workGroupSizeY = 32U;

UBO timeUBO {};
double timeCopy = 0.0;

void* mappedMemory = nullptr;

/**
 * Compute operations are going to need their own command buffer since there are two separate queue families
 * for graphics and compute operations
 */

void computeShaderSetup(ResourceManager* resourceManager) {

    VulkanApp* app = resourceManager->app;

    /**
     * First we create the two resources bound to the pipeline; The UBO and the image.
     */

    VkBufferCreateInfo uboCreateInfo{};
    uboCreateInfo.pNext = nullptr;
    uboCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    uboCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    uboCreateInfo.pQueueFamilyIndices = nullptr;
    uboCreateInfo.queueFamilyIndexCount = 0U;
    uboCreateInfo.size = sizeof(UBO);
    uboCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

    THROW(vkCreateBuffer(resourceManager->app->logicalDevice, &uboCreateInfo, nullptr, &computeUBO), "Failed to create UBO");

    VkMemoryRequirements uboMemReqs;
    vkGetBufferMemoryRequirements(app->logicalDevice, computeUBO, &uboMemReqs);
    uint32_t uboMemIndex = resourceManager->getSuitableMemoryTypeIndex(uboMemReqs, VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    VkMemoryAllocateInfo uboAllocInfo{};
    uboAllocInfo.pNext = nullptr;
    uboAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    uboAllocInfo.memoryTypeIndex = uboMemIndex;
    uboAllocInfo.allocationSize = uboMemReqs.size;

    THROW(vkAllocateMemory(app->logicalDevice, &uboAllocInfo, nullptr, &computeUBOMemory), "Failed to allocate UBO memory");

    THROW(vkBindBufferMemory(app->logicalDevice, computeUBO, computeUBOMemory, 0U), "Failed to bind UBO memory");

    outputImage = resourceManager->createImageAll(1024U, 1024U, AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE);


    computeShaderDescriptorPool = resourceManager->createDescriptorPool(1U, {{AppDescriptorItemTemplate::CS_UNIFORM_BUFFER, 1}, {AppDescriptorItemTemplate::CS_STORAGE_IMAGE, 1U}});

    /**
     * Next we create the descriptor set layout, which defines the descriptor set for the entirety of the pipeline
     */

    descriptorSetlayout = resourceManager->createDescriptorSetLayout({AppDescriptorItemTemplate::CS_UNIFORM_BUFFER, AppDescriptorItemTemplate::CS_STORAGE_IMAGE});

    /**
     * Next, we allocate the descriptor set itself from the descriptor pool
     */

    computePipelineDescriptorSet = resourceManager->allocateDescriptorSet(descriptorSetlayout, computeShaderDescriptorPool);

    /**
     * Next we create the compute shader module
     */

    std::vector<char> shaderCode = readFile("../shaders/build/comp.spv");
    VkShaderModuleCreateInfo compShaderModuleCreateInfo{};
    compShaderModuleCreateInfo.pNext = nullptr;
    compShaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    compShaderModuleCreateInfo.flags = 0U;
    compShaderModuleCreateInfo.pCode = reinterpret_cast<uint32_t*>(shaderCode.data());
    compShaderModuleCreateInfo.codeSize = shaderCode.size();

    THROW(vkCreateShaderModule(app->logicalDevice, &compShaderModuleCreateInfo, nullptr, &computeShaderModule), "Failed to create compute shader module");
    

    VkPipelineShaderStageCreateInfo shaderStageCreateInfo{};
    shaderStageCreateInfo.pNext = nullptr;
    shaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStageCreateInfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
    shaderStageCreateInfo.flags = 0U;
    shaderStageCreateInfo.module = computeShaderModule;
    shaderStageCreateInfo.pName = "main";
    shaderStageCreateInfo.pSpecializationInfo = nullptr;

    VkPipelineLayoutCreateInfo computePipelineLayoutCreateInfo{};
    computePipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    computePipelineLayoutCreateInfo.pNext = nullptr;
    computePipelineLayoutCreateInfo.pSetLayouts = &descriptorSetlayout;
    computePipelineLayoutCreateInfo.setLayoutCount = 1U;
    computePipelineLayoutCreateInfo.pPushConstantRanges = nullptr;
    computePipelineLayoutCreateInfo.pushConstantRangeCount = 0U;
    computePipelineLayoutCreateInfo.flags = 0U;

    THROW(vkCreatePipelineLayout(app->logicalDevice, &computePipelineLayoutCreateInfo, nullptr, &computePipelineLayout), "Failed to create compute pipeline layout");

    /**
     * Next, we create the compute pipeline
     */
    VkComputePipelineCreateInfo computePipelineCreateInfo{};
    computePipelineCreateInfo.pNext = nullptr;
    computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
    computePipelineCreateInfo.stage = shaderStageCreateInfo;
    computePipelineCreateInfo.basePipelineHandle = nullptr;
    computePipelineCreateInfo.basePipelineIndex = -1;
    computePipelineCreateInfo.stage = shaderStageCreateInfo;
    computePipelineCreateInfo.layout = computePipelineLayout;

    THROW(vkCreateComputePipelines(app->logicalDevice, nullptr, 1U, &computePipelineCreateInfo, nullptr, &computePipeline), "Failed to create compute pipeline");


    /**
     * Next, we create a command pool & allocate a command buffer to enqueue our compute shader commands
     */

    VkCommandPoolCreateInfo commandPoolCreateInfo{};
    commandPoolCreateInfo.pNext = nullptr;
    commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolCreateInfo.queueFamilyIndex = app->queueIndices.compute;
    commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // enable ability to reset buffer individually
    THROW(vkCreateCommandPool(app->logicalDevice, &commandPoolCreateInfo, nullptr, &computeCommandPool), "Failed to create command pool");


    VkCommandBufferAllocateInfo commandBufferAllocInfo{};
    commandBufferAllocInfo.pNext = nullptr;
    commandBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    commandBufferAllocInfo.commandPool = computeCommandPool;
    commandBufferAllocInfo.commandBufferCount = 1;
    commandBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;

    THROW(vkAllocateCommandBuffers(app->logicalDevice, &commandBufferAllocInfo, &computeCommandBuffer), "Failed to allocate command buffer");


    /**
     * We now create a fence to synchronize between the CPU and GPU, the GPU must first finish the compute shader operation before
     * the CPU can begin rendering
     */

    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.pNext = nullptr;
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = 0U; 

    THROW(vkCreateFence(app->logicalDevice, &fenceCreateInfo, nullptr, &computeShaderBusyFence), "Failed to create fence");

    /**
     * We now update the descriptor set that we created for this pipeline
     */

    resourceManager->updateBufferDescriptor(computePipelineDescriptorSet, computeUBO, sizeof(UBO), 0U, AppDescriptorItemTemplate::CS_UNIFORM_BUFFER);
    resourceManager->updateImageDescriptor(outputImage.imageView, computePipelineDescriptorSet, 1U, AppDescriptorItemTemplate::CS_STORAGE_IMAGE);

    /**
     * We must now transition the image layout for the output image to VK_IMAGE_LAYOUT_GENERAL
     * in preparation for its usage
     */

    VkImageMemoryBarrier memBar{};
    memBar.image = outputImage.image.get();
    memBar.pNext = nullptr;
    memBar.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memBar.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    memBar.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    memBar.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // We are not tranferring ownership from queue families
    memBar.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // ^..
    memBar.subresourceRange.levelCount = 1U;
    memBar.subresourceRange.layerCount = 1U;
    memBar.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    memBar.subresourceRange.baseArrayLayer = 0U;
    memBar.subresourceRange.baseMipLevel = 0U;
    memBar.srcAccessMask = 0U; // We are not waiting for anything that has occurred before the barrier, the image is in an undefined state
    memBar.dstAccessMask = VK_ACCESS_MEMORY_WRITE_BIT; // Memory writes must wait for this barrier
    
    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;
    THROW(vkBeginCommandBuffer(computeCommandBuffer, &beginInfo), "Failed to begin recording command buffer");

    vkCmdPipelineBarrier(computeCommandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
    0U, 0U, nullptr, 0U, nullptr, 1U, &memBar);

    THROW(vkEndCommandBuffer(computeCommandBuffer), "Failed to stop recording command buffer");

    VkSubmitInfo submitInfo{};
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.signalSemaphoreCount = 0U;

    vkQueueSubmit(app->computeQueue, 1U, &submitInfo, computeShaderBusyFence);

    vkWaitForFences(app->logicalDevice, 1U, &computeShaderBusyFence, true, UINT32_MAX);
    vkResetFences(app->logicalDevice, 1U, &computeShaderBusyFence);


    /** 
     * We now map the UBO device memory to host memory,
     * this memory will remain mapped until cleanup
     */

    THROW(vkMapMemory(app->logicalDevice, computeUBOMemory, 0U, sizeof(UBO), 0U, &mappedMemory), "Failed to map memory");
    
}

void computeShaderWriteCommandBuffer(ResourceManager *resourceManager)
{
    VulkanApp* app = resourceManager->app;
    timeCopy += app->deltaTime.count();
    timeUBO.time = static_cast<float>(timeCopy);
    memcpy(mappedMemory, &timeUBO, sizeof(UBO));

    VkCommandBufferBeginInfo beginInfo {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;
    THROW(vkBeginCommandBuffer(computeCommandBuffer, &beginInfo), "Failed to begin recording command buffer");

    vkCmdBindDescriptorSets(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipelineLayout, 0U, 1U, &computePipelineDescriptorSet,
    0U, nullptr);

    vkCmdBindPipeline(computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline);

    vkCmdDispatch(computeCommandBuffer, 1024U / workGroupSizeX, 1024U / workGroupSizeY, 1024U / workGroupSizeX);

    THROW(vkEndCommandBuffer(computeCommandBuffer), "Failed to stop recording command buffer");

    VkSubmitInfo submitInfo{};
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pCommandBuffers = &computeCommandBuffer;
    submitInfo.commandBufferCount = 1U;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.waitSemaphoreCount = 0U;
    submitInfo.signalSemaphoreCount = 0U;

    // The compute shader fence is signalled once the operation is complete
    THROW(vkQueueSubmit(app->computeQueue, 1U, &submitInfo, computeShaderBusyFence), "Failed to submit command buffer to queue");

    vkWaitForFences(app->logicalDevice, 1U, &computeShaderBusyFence, true, UINT32_MAX);
    vkResetFences(app->logicalDevice, 1U, &computeShaderBusyFence);
    
    /**
     * This will run at the start of recording the command buffer, after the render pass has started
     */ 
}

void computeShaderCleanup(ResourceManager* resourceManager)
{
    VulkanApp* app = resourceManager->app;
    vkUnmapMemory(app->logicalDevice, computeUBOMemory);
    vkDeviceWaitIdle(app->logicalDevice);
    vkDestroyFence(app->logicalDevice, computeShaderBusyFence, nullptr);
    vkFreeCommandBuffers(app->logicalDevice, computeCommandPool, 1U, &computeCommandBuffer);
    vkDestroyCommandPool(app->logicalDevice, computeCommandPool, nullptr);
    vkDestroyPipeline(app->logicalDevice, computePipeline, nullptr);
    vkDestroyPipelineLayout(app->logicalDevice, computePipelineLayout, nullptr);
    vkDestroyShaderModule(app->logicalDevice, computeShaderModule, nullptr);
    vkFreeMemory(app->logicalDevice, computeUBOMemory, nullptr);
    vkDestroyBuffer(app->logicalDevice, computeUBO, nullptr);
}

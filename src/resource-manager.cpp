#include "resource-manager.h"
#include "vulkan-app.h"

void ResourceManager::init(VulkanApp *appRef)
{
    this->app = appRef;
}

uint32_t ResourceManager::getSuitableMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags propertyFlags)
{
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(app->physicalDevice, &memoryProperties);
    
    // A bitmask where (1 << i) is set iff memoryProperties.memoryType[i] is supported given the requirements
    for (uint32_t index = 0 ; index < memoryProperties.memoryTypeCount ; index++) {
        // Is memory at index 'index' supported? (indicated by 1 in bitmask), does the memory at that index support all of the desired property flags? 
        if ((memoryRequirements.memoryTypeBits & (1 << index)) && ((memoryProperties.memoryTypes[index].propertyFlags & propertyFlags) == propertyFlags)) {
            return index;
        }
    }

    throw std::runtime_error("Unable to find a suitable memory type");
}


AppImage2D ResourceManager::createImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{

    AppImage2D returnImage{};
    // Instantiate image create info from template
    VkImageCreateInfo createInfo{ getImageCreateInfoFromTemplate(appImageTemplate) };

    // Fill in sizing data
    createInfo.extent.height = height;
    createInfo.extent.width = width;
    createInfo.arrayLayers = layerCount;

    // Create a new image object in the vector of images
    images.push_back(VkImage{});

    // Attempt to create the image
    THROW(vkCreateImage(app->logicalDevice, &createInfo, nullptr, &images.back()), "Failed to create image");

    returnImage.image = images.back();
    returnImage.imageCreationTemplate = appImageTemplate;
    returnImage.layerCount = layerCount;
    returnImage.memory = VK_NULL_HANDLE;
    returnImage.view = VK_NULL_HANDLE;
    returnImage.width = width;
    returnImage.height = height;
    returnImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    return returnImage;
}

void ResourceManager::createImageView(AppImage2D &image, uint32_t layerCount, uint32_t baseLayer)
{
    VkImageViewCreateInfo createInfo{ getImageViewCreateInfoFromTemplate(image.imageCreationTemplate) };
    createInfo.image = image.image;
    createInfo.subresourceRange.layerCount = layerCount;
    createInfo.subresourceRange.baseArrayLayer = baseLayer;

    imageViews.push_back(VkImageView{});

    THROW(vkCreateImageView(app->logicalDevice, &createInfo, nullptr, &imageViews.back()), "Failed to create image view");

    image.view = imageViews.back();
}

void ResourceManager::allocateImageMemory(AppImage2D &image)
{
    VkMemoryPropertyFlags memoryPropertyFlags = 0U;

    switch (image.imageCreationTemplate) {
        case AppImageTemplate::DEPTH_STENCIL :
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;

        case AppImageTemplate::STAGING_IMAGE_TEXTURE : {
            memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        }
    }

    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(app->logicalDevice, image.image, &imageMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(imageMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = imageMemoryRequirements.size;

    imageMemory.push_back(VkDeviceMemory{});

    THROW(vkAllocateMemory(app->logicalDevice, &allocInfo, nullptr, &imageMemory.back()), "Failed to allocate image memory");

    image.memory = imageMemory.back();
}

void ResourceManager::bindImageToMemory(AppImage2D &image)
{
    THROW(vkBindImageMemory(app->logicalDevice, image.image, image.memory, 0U), "Failed to bind image to memory");
}

AppImage2D ResourceManager::createImageAll(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{
    AppImage2D image;
    image = createImage(width, height, appImageTemplate, layerCount);
    allocateImageMemory(image);
    bindImageToMemory(image);
    createImageView(image);
    return image;
}

void ResourceManager::transitionImageLayout(AppImage2D &image, VkImageLayout newLayout)
{
    VkImageMemoryBarrier layoutTransitionBarrier{};
    layoutTransitionBarrier.pNext = nullptr;
    layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //                                {aspect mask, mip level, mip level count, array layer, array layer count}
    layoutTransitionBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 1U, 0U, 1U};
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.srcAccessMask = VK_ACCESS_NONE; // We are not waiting for anything to occur prior to this barrier
    layoutTransitionBarrier.dstAccessMask = VK_ACCESS_NONE; // Nothing is awaiting this barrier
    layoutTransitionBarrier.image = image.image;
    layoutTransitionBarrier.oldLayout = image.imageLayout;
    layoutTransitionBarrier.newLayout = newLayout;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;

    vkBeginCommandBuffer(app->commandBuffer, &beginInfo);
    vkCmdPipelineBarrier(app->commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        0U, 0U, nullptr, 0U, nullptr, 1U, &layoutTransitionBarrier);
    vkEndCommandBuffer(app->commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice, &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = &(app->commandBuffer);
    submitInfo.commandBufferCount = 1U;
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0u;

    vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice, 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice);
    
    vkDestroyFence(app->logicalDevice, transferCompleteFence, nullptr);

    image.imageLayout = newLayout;
}

void ResourceManager::pushStagingImage(AppImage2D &stagingImage, AppImage2D &deviceLocalImage)
{
    transitionImageLayout(stagingImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    transitionImageLayout(deviceLocalImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = &(app->commandBuffer);
    submitInfo.commandBufferCount = 1U;
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0u;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;

    VkImageCopy imgCopy{};
    //                  {x, y, z}
    imgCopy.srcOffset = {0U, 0U, 0U};
    imgCopy.dstOffset = {0U, 0U, 0U};
    imgCopy.extent.width = stagingImage.width;
    imgCopy.extent.height = stagingImage.height;
    imgCopy.extent.depth = 1U; 
    //                       {Aspect, Mip level, Array layer, Layer count}
    imgCopy.srcSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 0U, 1U};
    imgCopy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 0U, 1U};

    vkBeginCommandBuffer(app->commandBuffer, &beginInfo);
    vkCmdCopyImage(app->commandBuffer, stagingImage.image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, deviceLocalImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &imgCopy);
    vkEndCommandBuffer(app->commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice, &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice, 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice);
    
    vkDestroyFence(app->logicalDevice, transferCompleteFence, nullptr);
}

AppBuffer ResourceManager::createBuffer(AppBufferTemplate bufferTemplate, size_t size)
{
    AppBuffer returnBuffer {};
    returnBuffer.appBufferTemplate = bufferTemplate;
    returnBuffer.size = size;

    VkBufferCreateInfo createInfo{getBufferCreateInfoFromTemplate(bufferTemplate)};
    createInfo.size = size;

    buffers.push_back(VkBuffer{});

    THROW(vkCreateBuffer(app->logicalDevice, &createInfo, nullptr, &buffers.back()), "Failed to create buffer"); 
    returnBuffer.buffer = buffers.back();

    return returnBuffer;
}

void ResourceManager::allocateBufferMemory(AppBuffer &appBuffer)
{
    VkMemoryPropertyFlags memoryPropertyFlags = 0U;

    switch (appBuffer.appBufferTemplate) {
        case AppBufferTemplate::UNIFORM_BUFFER :
        case AppBufferTemplate::VERTEX_BUFFER_STAGING :
        case AppBufferTemplate::INDEX_BUFFER_STAGING :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
            break;
        case AppBufferTemplate::VERTEX_BUFFER_DEVICE :
        case AppBufferTemplate::INDEX_BUFFER_DEVICE :
            memoryPropertyFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            break;
    };

    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(app->logicalDevice, appBuffer.buffer, &bufferMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(bufferMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;

    bufferMemory.push_back(VkDeviceMemory{});

    THROW(vkAllocateMemory(app->logicalDevice, &memoryAllocateInfo, nullptr, &bufferMemory.back()), "Failed to allocate buffer memory");

    appBuffer.memory = bufferMemory.back();
}

void ResourceManager::bindBufferToMemory(AppBuffer &appBuffer)
{
    THROW(vkBindBufferMemory(app->logicalDevice, appBuffer.buffer, appBuffer.memory, 0U), "Failed to bind buffer to memory");
}

AppBuffer ResourceManager::createBufferAll(AppBufferTemplate bufferTemplate, size_t size)
{
    AppBuffer buffer{};
    buffer = createBuffer(bufferTemplate, size);
    allocateBufferMemory(buffer);
    bindBufferToMemory(buffer);
    return buffer;
}

void ResourceManager::pushStagingBuffer(AppBuffer &stagingBuffer, AppBuffer &deviceLocalBuffer)
{
    VkSubmitInfo submitInfo{};
    submitInfo.pCommandBuffers = &(app->commandBuffer);
    submitInfo.commandBufferCount = 1U;
    submitInfo.pNext = nullptr;
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pSignalSemaphores = nullptr;
    submitInfo.signalSemaphoreCount = 0U;
    submitInfo.pWaitSemaphores = nullptr;
    submitInfo.pWaitDstStageMask = nullptr;
    submitInfo.waitSemaphoreCount = 0u;

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.pNext = nullptr;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pInheritanceInfo = nullptr;
    beginInfo.flags = 0U;

    VkBufferCopy bufferCopy{};
    //                  {x, y, z}
    bufferCopy.dstOffset = 0U;
    bufferCopy.srcOffset = 0U;
    bufferCopy.size = stagingBuffer.size;

    vkBeginCommandBuffer(app->commandBuffer, &beginInfo);
    vkCmdCopyBuffer(app->commandBuffer, stagingBuffer.buffer, deviceLocalBuffer.buffer, 1U, &bufferCopy);
    vkEndCommandBuffer(app->commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice, &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice, 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice);
    
    vkDestroyFence(app->logicalDevice, transferCompleteFence, nullptr);
}

void ResourceManager::copyDataToStagingBuffer(AppBuffer &stagingBuffer, void *data, size_t size)
{
    void* mappedMemory = nullptr;
    vkMapMemory(app->logicalDevice, stagingBuffer.memory, 0U, size, 0U, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(app->logicalDevice, stagingBuffer.memory);
}

VkDescriptorSetLayout ResourceManager::createDescriptorSetLayout(std::vector<AppDescriptorItemTemplate> descriptorItems)
{
    uint32_t index = 0U;
    std::vector<VkDescriptorSetLayoutBinding> layoutBindings = {};
    for (AppDescriptorItemTemplate descriptorItem : descriptorItems) {
        VkDescriptorType descriptorType = getDescriptorTypeFromTemplate(descriptorItem);
        if (layoutBindings.size() > 0 && layoutBindings[index - 1].descriptorType == descriptorType && false)
            layoutBindings[index - 1].descriptorCount++;
        else {
             /**
             * 
             * binding: Set to 0, this corresponds to the binding number of the UBO that we will be binding in the shader.
             * descriptorCount: We are only binding and creating 1 descriptor in this case. 
             * descriptorType: The type of descriptors that this descriptor set consists of (set for UBOs).
             * stageFlags: Set to the vertex shader since we will be accessing the UBO in the vertex shader.
             * pImmutableSamplers: Used if this descriptor set is a sampler resource (nullptr because UBO not sampler).
             */
            layoutBindings.push_back(VkDescriptorSetLayoutBinding{});
            layoutBindings.back().binding = index;
            layoutBindings.back().descriptorCount = 1U;
            layoutBindings.back().stageFlags = getDescriptorStageFlagFromTemplate(descriptorItem);
            layoutBindings.back().descriptorType = descriptorType;
            layoutBindings.back().pImmutableSamplers = nullptr;
        }
        index++;
    }

    VkDescriptorSetLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.bindingCount = layoutBindings.size();
    createInfo.pBindings = layoutBindings.data();
    createInfo.flags = 0U;

    descriptorSetLayouts.push_back(VkDescriptorSetLayout{});

    THROW(vkCreateDescriptorSetLayout(app->logicalDevice, &createInfo, nullptr, &descriptorSetLayouts.back()), "Failed to create descriptor set layout");

    return descriptorSetLayouts.back();
}

VkDescriptorPool ResourceManager::createDescriptorPool(uint32_t maxSetsCount, std::map<AppDescriptorItemTemplate, uint32_t> descriptorTypeCounts)
{
    std::vector<VkDescriptorPoolSize> poolSizes = {};
    for (auto descriptorTypeCount = descriptorTypeCounts.begin() ; descriptorTypeCount != descriptorTypeCounts.end() ; descriptorTypeCount++) {
        VkDescriptorType type = getDescriptorTypeFromTemplate(descriptorTypeCount->first);
        uint32_t count = descriptorTypeCount->second;
        poolSizes.push_back(VkDescriptorPoolSize{type, count});
    }

    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.pNext = nullptr;
    createInfo.poolSizeCount = poolSizes.size();
    createInfo.pPoolSizes = poolSizes.data();
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = maxSetsCount;

    descriptorPools.push_back(VkDescriptorPool{});

    THROW(vkCreateDescriptorPool(app->logicalDevice, &createInfo, nullptr, &descriptorPools.back()), "Failed to create descriptor pool");

    return descriptorPools.back();
}

VkDescriptorSet ResourceManager::allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool)
{
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.pSetLayouts = &descriptorSetLayout;
    allocInfo.descriptorSetCount = 1U;
    allocInfo.descriptorPool = descriptorPool;

    descriptorSets.push_back(VkDescriptorSet{});

    THROW(vkAllocateDescriptorSets(app->logicalDevice, &allocInfo, &descriptorSets.back()), "Failed to allocate descriptor set");

    return descriptorSets.back();
}

void ResourceManager::updateImageDescriptor(AppImage2D image, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, VkSampler sampler)
{   
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = getImageLayoutFromTemplate(image.imageCreationTemplate);
    imageInfo.imageView = image.view;
    imageInfo.sampler = sampler;

    VkWriteDescriptorSet descriptorWriteImg{};
    descriptorWriteImg.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteImg.pNext = nullptr;
    descriptorWriteImg.descriptorCount = 1;
    descriptorWriteImg.pBufferInfo = nullptr;
    descriptorWriteImg.descriptorType = getDescriptorTypeFromTemplate(itemTemplate);
    descriptorWriteImg.dstSet = set;
    descriptorWriteImg.dstBinding = binding;
    descriptorWriteImg.dstArrayElement = 0;
    descriptorWriteImg.pImageInfo = &imageInfo;
    descriptorWriteImg.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(app->logicalDevice, 1U, &descriptorWriteImg, 0U, nullptr);
}

void ResourceManager::updateBufferDescriptor(VkDescriptorSet set, VkBuffer buffer, uint32_t size, uint32_t binding, AppDescriptorItemTemplate itemTemplate)
{
    VkDescriptorBufferInfo bufferInfo{};
    bufferInfo.buffer = buffer;
    bufferInfo.offset = 0;
    bufferInfo.range = size;

    VkWriteDescriptorSet descriptorWriteBuffer{};
    descriptorWriteBuffer.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptorWriteBuffer.pNext = nullptr;
    descriptorWriteBuffer.descriptorCount = 1;
    descriptorWriteBuffer.pBufferInfo = &bufferInfo;
    descriptorWriteBuffer.descriptorType = getDescriptorTypeFromTemplate(itemTemplate);
    descriptorWriteBuffer.dstSet = set;
    descriptorWriteBuffer.dstBinding = binding;
    descriptorWriteBuffer.dstArrayElement = 0;
    descriptorWriteBuffer.pImageInfo = nullptr;
    descriptorWriteBuffer.pTexelBufferView = nullptr;

    vkUpdateDescriptorSets(app->logicalDevice, 1U, &descriptorWriteBuffer, 0U, nullptr);
}

void ResourceManager::destroy()
{
    for (VkBuffer buffer : buffers)
        vkDestroyBuffer(app->logicalDevice, buffer, nullptr);

    for (VkDeviceMemory memory : bufferMemory)
        vkFreeMemory(app->logicalDevice, memory, nullptr);

    // Destroy the image views first
    for (VkImageView imageView : imageViews) 
        vkDestroyImageView(app->logicalDevice, imageView, nullptr);

    // Destroy images next
    for (VkImage image : images) 
        vkDestroyImage(app->logicalDevice, image, nullptr);

    // Finally, destroy image memory
    for (VkDeviceMemory memoryResource : imageMemory) 
        vkFreeMemory(app->logicalDevice, memoryResource, nullptr);

    for (VkDescriptorPool descriptorPool : descriptorPools)
        vkDestroyDescriptorPool(app->logicalDevice, descriptorPool, nullptr);

    for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(app->logicalDevice, descriptorSetLayout, nullptr);
}

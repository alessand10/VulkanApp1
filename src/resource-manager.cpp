#include "resource-manager.h"
#include "vulkan-app.h"
#include "utilities.h"
#include "GLFW/glfw3.h"

AppInstance ResourceManager::createInstance(const char* appName, bool enableValidationLayers)
{
    AppInstance returnInstance{};

    const std::vector<const char*> validationLayers = {
        "VK_LAYER_KHRONOS_validation"
    };

    VkApplicationInfo appCreateInfo{};
    appCreateInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appCreateInfo.pApplicationName = appName;
    appCreateInfo.applicationVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.pEngineName = "No Engine";
    appCreateInfo.engineVersion = VK_MAKE_API_VERSION(1, 1, 0, 0);
    appCreateInfo.apiVersion = VK_API_VERSION_1_3;

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

    instances.push_front({});
    returnInstance.setRef(instances.begin());

    THROW(vkCreateInstance(&instanceCreateInfo, nullptr, &instances.front()), "Failed to create Vulkan instance");

    return returnInstance;
}

AppSurface ResourceManager::createSurface(AppInstance instance, GLFWwindow *window)
{
    AppSurface returnSurface{};
    surfaces.push_front({});
    returnSurface.setRef(surfaces.begin());

    THROW(glfwCreateWindowSurface(instance.get(), window, nullptr, &surfaces.front()), "Failed to create window surface");

    return returnSurface;
}

AppDevice ResourceManager::createDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices)
{
    AppDevice returnDevice {};

    uint32_t queueFamilyCount = 0;

    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> familyProperties(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, familyProperties.data());

    int index = 0;
    for (VkQueueFamilyProperties family : familyProperties) {
        if (family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            queueFamilyIndices->graphics = index;
        else if (family.queueFlags & VK_QUEUE_COMPUTE_BIT)
            queueFamilyIndices->compute = index;
        else if (family.queueFlags & VK_QUEUE_TRANSFER_BIT)
            queueFamilyIndices->transfer = queueFamilyIndices->graphics;
        index++;
    }

    float queuePriority = 1.0f;

    VkDeviceQueueCreateInfo deviceGraphicsQueueCreateInfo{};
    deviceGraphicsQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceGraphicsQueueCreateInfo.queueCount = 1;
    deviceGraphicsQueueCreateInfo.queueFamilyIndex = queueFamilyIndices->graphics;
    deviceGraphicsQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceGraphicsQueueCreateInfo.pNext = NULL;

    VkDeviceQueueCreateInfo deviceComputeQueueCreateInfo{};
    deviceComputeQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    deviceComputeQueueCreateInfo.queueCount = 1;
    deviceComputeQueueCreateInfo.queueFamilyIndex = queueFamilyIndices->compute;
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

    devices.push_front({});
    returnDevice.setRef(devices.begin());

    THROW(vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &devices.front()), "Failed to create device");

    return returnDevice;
}

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


AppImage ResourceManager::createImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{

    AppImage returnImage{};
    // Instantiate image create info from template
    VkImageCreateInfo createInfo{ getImageCreateInfoFromTemplate(appImageTemplate) };

    // Fill in sizing data
    createInfo.extent.height = height;
    createInfo.extent.width = width;
    createInfo.arrayLayers = layerCount;

    // Create a new image object in the vector of images
    images.push_front(VkImage{});
    returnImage.setRef(images.begin());

    // Attempt to create the image
    THROW(vkCreateImage(app->logicalDevice.get(), &createInfo, nullptr, &images.front()), "Failed to create image");

    returnImage.imageCreationTemplate = appImageTemplate;
    returnImage.layerCount = layerCount;
    returnImage.width = width;
    returnImage.height = height;
    returnImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    return returnImage;
}

AppImage ResourceManager::addExistingImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers, VkImageLayout layout, VkImage image)
{
    AppImage returnImage{};
    images.push_front(image);
    returnImage.setRef(images.begin());
    returnImage.imageCreationTemplate = appImageTemplate;
    returnImage.layerCount = layers;
    returnImage.width = width;
    returnImage.imageLayout = layout;
    returnImage.height = height;
    returnImage.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    return returnImage;
}

void ResourceManager::destroyImage(AppImage image)
{
    // Destroy the Vulkan resource
    vkDestroyImage(app->logicalDevice.get(), image.get(), nullptr);

    // Destroy the reference
    images.erase(image.getRef());
}

AppImageView ResourceManager::createImageView(AppImage &image, uint32_t layerCount, uint32_t baseLayer)
{
    AppImageView returnView {};
    returnView.imageCreationTemplate = image.imageCreationTemplate;

    VkImageViewCreateInfo createInfo{ getImageViewCreateInfoFromTemplate(image.imageCreationTemplate) };
    createInfo.image = image.get();
    createInfo.subresourceRange.layerCount = layerCount;
    createInfo.subresourceRange.baseArrayLayer = baseLayer;

    imageViews.push_front(VkImageView{});
    returnView.setRef(imageViews.begin());

    THROW(vkCreateImageView(app->logicalDevice.get(), &createInfo, nullptr, &imageViews.front()), "Failed to create image view");
    return returnView;
}

void ResourceManager::destroyImageView(AppImageView &imageView)
{
    // Destroy the view
    vkDestroyImageView(app->logicalDevice.get(), imageView.get(), nullptr);

    // Destroy the node
    imageViews.erase(imageView.getRef());
}

AppDeviceMemory ResourceManager::allocateImageMemory(AppImage &image)
{
    AppDeviceMemory returnMemory{};
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
    vkGetImageMemoryRequirements(app->logicalDevice.get(), image.get(), &imageMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(imageMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.memoryTypeIndex = memoryTypeIndex;
    allocInfo.allocationSize = imageMemoryRequirements.size;

    deviceMemorySet.push_front(VkDeviceMemory{});
    returnMemory.setRef(deviceMemorySet.begin());

    THROW(vkAllocateMemory(app->logicalDevice.get(), &allocInfo, nullptr, &deviceMemorySet.front()), "Failed to allocate image memory");
    return returnMemory;
}

void ResourceManager::bindImageToMemory(AppImage &image, AppDeviceMemory &deviceMemory)
{
    THROW(vkBindImageMemory(app->logicalDevice.get(), image.get(), deviceMemory.get(), 0U), "Failed to bind image to memory");
}

void ResourceManager::destroyDeviceMemory(AppDeviceMemory deviceMemory)
{
    vkFreeMemory(app->logicalDevice.get(), deviceMemory.get(), nullptr);
    deviceMemorySet.erase(deviceMemory.getRef());
}

AppImageBundle ResourceManager::createImageAll(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount)
{
    AppImageBundle bundle {};
    bundle.image = createImage(width, height, appImageTemplate, layerCount);
    bundle.deviceMemory = allocateImageMemory(bundle.image);
    bindImageToMemory(bundle.image, bundle.deviceMemory);
    bundle.imageView = createImageView(bundle.image);
    return bundle;
}

void ResourceManager::transitionImageLayout(AppImage &image, VkImageLayout newLayout, uint32_t targetLayer)
{
    VkImageMemoryBarrier layoutTransitionBarrier{};
    layoutTransitionBarrier.pNext = nullptr;
    layoutTransitionBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    //                                {aspect mask, mip level, mip level count, array layer, array layer count}
    layoutTransitionBarrier.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, 1U, targetLayer, 1U};
    layoutTransitionBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    layoutTransitionBarrier.srcAccessMask = VK_ACCESS_NONE; // We are not waiting for anything to occur prior to this barrier
    layoutTransitionBarrier.dstAccessMask = VK_ACCESS_NONE; // Nothing is awaiting this barrier
    layoutTransitionBarrier.image = image.get();
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
    vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

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

    vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice.get());
    
    vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);

    image.imageLayout = newLayout;
}

void ResourceManager::pushStagingImage(AppImage &stagingImage, AppImage &deviceLocalImage, uint32_t deviceLocalLayer)
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
    imgCopy.dstSubresource = {VK_IMAGE_ASPECT_COLOR_BIT, 0U, deviceLocalLayer, 1U};

    vkBeginCommandBuffer(app->commandBuffer, &beginInfo);
    vkCmdCopyImage(app->commandBuffer, stagingImage.get(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, deviceLocalImage.get(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1U, &imgCopy);
    vkEndCommandBuffer(app->commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice.get());
    
    vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);
}

AppBuffer ResourceManager::createBuffer(AppBufferTemplate bufferTemplate, size_t size)
{
    AppBuffer returnBuffer {};
    returnBuffer.appBufferTemplate = bufferTemplate;
    returnBuffer.size = size;

    VkBufferCreateInfo createInfo{getBufferCreateInfoFromTemplate(bufferTemplate)};
    createInfo.size = size;

    buffers.push_front(VkBuffer{});
    returnBuffer.setRef(buffers.begin());

    THROW(vkCreateBuffer(app->logicalDevice.get(), &createInfo, nullptr, &buffers.front()), "Failed to create buffer"); 

    return returnBuffer;
}

AppDeviceMemory ResourceManager::allocateBufferMemory(AppBuffer &appBuffer)
{
    AppDeviceMemory returnDeviceMemory{};
    
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
    vkGetBufferMemoryRequirements(app->logicalDevice.get(), appBuffer.get(), &bufferMemoryRequirements);
    uint32_t memoryTypeIndex = getSuitableMemoryTypeIndex(bufferMemoryRequirements, memoryPropertyFlags);

    VkMemoryAllocateInfo memoryAllocateInfo {};
    memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    memoryAllocateInfo.pNext = nullptr;
    memoryAllocateInfo.memoryTypeIndex = memoryTypeIndex;
    memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;

    deviceMemorySet.push_front(VkDeviceMemory{});
    returnDeviceMemory.setRef(deviceMemorySet.begin());

    THROW(vkAllocateMemory(app->logicalDevice.get(), &memoryAllocateInfo, nullptr, &deviceMemorySet.front()), "Failed to allocate buffer memory");
    return returnDeviceMemory;
}

void ResourceManager::bindBufferToMemory(AppBuffer &buffer, AppDeviceMemory &deviceMemory)
{
    THROW(vkBindBufferMemory(app->logicalDevice.get(), buffer.get(), deviceMemory.get(), 0U), "Failed to bind buffer to memory");
}

AppBufferBundle ResourceManager::createBufferAll(AppBufferTemplate bufferTemplate, size_t size)
{
    AppBufferBundle bufferBundle {};
    bufferBundle.buffer = createBuffer(bufferTemplate, size);
    bufferBundle.deviceMemory = allocateBufferMemory(bufferBundle.buffer);
    bindBufferToMemory(bufferBundle.buffer, bufferBundle.deviceMemory);
    return bufferBundle;
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
    vkCmdCopyBuffer(app->commandBuffer, stagingBuffer.get(), deviceLocalBuffer.get(), 1U, &bufferCopy);
    vkEndCommandBuffer(app->commandBuffer);

    VkFenceCreateInfo transferCompleteFenceInfo{};
    transferCompleteFenceInfo.pNext = nullptr;
    transferCompleteFenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;

    VkFence transferCompleteFence;
    vkCreateFence(app->logicalDevice.get(), &transferCompleteFenceInfo, nullptr, &transferCompleteFence);

    vkQueueSubmit(app->graphicsQueue, 1U, &submitInfo, transferCompleteFence);

    vkWaitForFences(app->logicalDevice.get(), 1U, &transferCompleteFence, true, UINT32_MAX);

    vkDeviceWaitIdle(app->logicalDevice.get());
    
    vkDestroyFence(app->logicalDevice.get(), transferCompleteFence, nullptr);
}

void ResourceManager::copyDataToStagingMemory(AppDeviceMemory &stagingMemory, void *data, size_t size)
{
    void* mappedMemory = nullptr;
    vkMapMemory(app->logicalDevice.get(), stagingMemory.get(), 0U, size, 0U, &mappedMemory);
    memcpy(mappedMemory, data, size);
    vkUnmapMemory(app->logicalDevice.get(), stagingMemory.get());
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

    THROW(vkCreateDescriptorSetLayout(app->logicalDevice.get(), &createInfo, nullptr, &descriptorSetLayouts.back()), "Failed to create descriptor set layout");

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

    THROW(vkCreateDescriptorPool(app->logicalDevice.get(), &createInfo, nullptr, &descriptorPools.back()), "Failed to create descriptor pool");

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

    THROW(vkAllocateDescriptorSets(app->logicalDevice.get(), &allocInfo, &descriptorSets.back()), "Failed to allocate descriptor set");

    return descriptorSets.back();
}

void ResourceManager::updateImageDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, AppSampler sampler)
{   
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = getImageLayoutFromTemplate(imageView.imageCreationTemplate);
    imageInfo.imageView = imageView.get();
    imageInfo.sampler = sampler.get();

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

    vkUpdateDescriptorSets(app->logicalDevice.get(), 1U, &descriptorWriteImg, 0U, nullptr);
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

    vkUpdateDescriptorSets(app->logicalDevice.get(), 1U, &descriptorWriteBuffer, 0U, nullptr);
}

VkRenderPass ResourceManager::createRenderPass(std::vector<AppAttachment> attachments, std::vector<AppSubpass> subpasses, std::vector<VkSubpassDependency> subpassDependencies) {
    VkRenderPassCreateInfo renderPassInfo{};
    std::vector<VkAttachmentDescription> attachmentDescriptions = {};
    std::vector<VkSubpassDescription> subpassDescriptions = {};

    // Each std::vector<VkAttachmentReference> element is for a separate subpass
    std::vector<std::vector<VkAttachmentReference>> colorAttachmentReferences = {};
    std::vector<std::vector<VkAttachmentReference>> depthStencilAttachmentReferences = {};
    
    for (AppAttachment attachment : attachments) {
        attachmentDescriptions.push_back(getAttachmentDescriptionFromTemplate(attachment.attachmentTemplate));
    }

    for (AppSubpass subpass : subpasses) {
        colorAttachmentReferences.push_back(std::vector<VkAttachmentReference>{});
        depthStencilAttachmentReferences.push_back(std::vector<VkAttachmentReference>{});
        subpassDescriptions.push_back(VkSubpassDescription{});

        for (AppSubpassAttachmentRef ref : subpass.attachmentRefs) {
            AttachmentType attachmentType = getAttachmentTypeFromTemplate(attachments[ref.attachmentIndex].attachmentTemplate);
            if (attachmentType == AttachmentType::COLOR) {
                colorAttachmentReferences.back().push_back(VkAttachmentReference{});
                colorAttachmentReferences.back().back().attachment = ref.attachmentIndex;
                colorAttachmentReferences.back().back().layout = ref.imageLayout;
            }
            else if (attachmentType == AttachmentType::DEPTH_STENCIL) {
                depthStencilAttachmentReferences.back().push_back(VkAttachmentReference{});
                depthStencilAttachmentReferences.back().back().attachment = ref.attachmentIndex;
                depthStencilAttachmentReferences.back().back().layout = ref.imageLayout;
            }
        }

        subpassDescriptions.back().colorAttachmentCount = colorAttachmentReferences.size();
        subpassDescriptions.back().pColorAttachments = colorAttachmentReferences.back().size() == 0 ? nullptr : colorAttachmentReferences.back().data();
        subpassDescriptions.back().flags = 0U;
        subpassDescriptions.back().pDepthStencilAttachment = depthStencilAttachmentReferences.back().size() == 0 ? nullptr : &(depthStencilAttachmentReferences.back()[0]);

    }

    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pNext = nullptr;
    renderPassInfo.attachmentCount = attachments.size();
    renderPassInfo.pAttachments = attachmentDescriptions.data();
    renderPassInfo.subpassCount = subpassDescriptions.size();
    renderPassInfo.pSubpasses = subpassDescriptions.data();
    renderPassInfo.dependencyCount = subpassDependencies.size();
    renderPassInfo.pDependencies = subpassDependencies.data();
    renderPassInfo.flags = 0U;

    renderPasses.push_back(VkRenderPass{});
    THROW(vkCreateRenderPass(app->logicalDevice.get(), &renderPassInfo, nullptr, &renderPasses.back()), "Failed to create render pass");

    return renderPasses.back();
}

AppShaderModule ResourceManager::createShaderModule(std::string path, VkShaderStageFlagBits shaderStageFlags)
{
    std::vector<char> shaderFile = readFile(path.data());

    shaderModules.push_back(VkShaderModule{});

    VkShaderModuleCreateInfo shaderModuleCreateInfo{};
    shaderModuleCreateInfo.codeSize = shaderFile.size();
    shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderModuleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shaderFile.data());

    if (vkCreateShaderModule(app->logicalDevice.get(), &shaderModuleCreateInfo, NULL, &shaderModules.back()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex shader");
    }

    return AppShaderModule{shaderModules.back(), shaderStageFlags};
}

VkPipelineLayout ResourceManager::createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts)
{
    // Create the pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = descriptorSetLayouts.size(); // Optional
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data(); // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    pipelineLayouts.push_back(VkPipelineLayout{});

    THROW(vkCreatePipelineLayout(app->logicalDevice.get(), &pipelineLayoutInfo, NULL, &pipelineLayouts.back()), "Failed to create pipeline layout")

    return pipelineLayouts.back();
}

VkPipeline ResourceManager::createGraphicsPipeline(std::vector<AppShaderModule> shaderModules, VkPipelineLayout pipelineLayout, VkRenderPass renderPass)
{
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
    viewport.width = (float) app->windowWidth;
    viewport.height = (float) app->windowHeight;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    // Configure the scissor (can be used to discard rasterizer pixels)
    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent.width = app->windowWidth;
    scissor.extent.height = app->windowHeight;

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
        shaderStageCreateInfos.back().module = appShaderModule.shaderModule;
        shaderStageCreateInfos.back().pName = "main";
        shaderStageCreateInfos.back().stage = appShaderModule.shaderStage;
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
    colorSubpassPipelineInfo.layout = pipelineLayout;
    colorSubpassPipelineInfo.renderPass = renderPass;
    colorSubpassPipelineInfo.subpass = 0;
    colorSubpassPipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    colorSubpassPipelineInfo.basePipelineIndex = -1; // Optional
    
    pipelines.push_back(VkPipeline{});

    THROW(vkCreateGraphicsPipelines(app->logicalDevice.get(), VK_NULL_HANDLE, 1, &colorSubpassPipelineInfo, NULL, &pipelines.back()), "Failed to create graphics pipeline");

    return pipelines.back();
}

VkFence ResourceManager::createFence(bool signaled)
{
    fences.push_back(VkFence{});

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0U;

    THROW(vkCreateFence(app->logicalDevice.get(), &fenceInfo, NULL, &fences.back()), "Failed to create fence");
    return fences.back();
}

VkSemaphore ResourceManager::createSemaphore()
{
    semaphores.push_back(VkSemaphore{});

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    THROW(vkCreateSemaphore(app->logicalDevice.get(), &semaphoreInfo, NULL, &semaphores.back()), "Failed to create semaphore");
    return semaphores.back();
}

AppSwapchain ResourceManager::createSwapchain()
{
    AppSwapchain appSwapchain{};

    VkSurfaceCapabilitiesKHR surfaceCapabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app->physicalDevice, app->surface.get(), &surfaceCapabilities);

    VkSwapchainCreateInfoKHR swapchainCreateInfo{};
    swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    swapchainCreateInfo.surface = app->surface.get();
    swapchainCreateInfo.oldSwapchain = VK_NULL_HANDLE;
    swapchainCreateInfo.clipped = VK_FALSE;
    swapchainCreateInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR;
    swapchainCreateInfo.imageArrayLayers = 1;
    swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    swapchainCreateInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    swapchainCreateInfo.imageFormat = VK_FORMAT_B8G8R8A8_SRGB;
    swapchainCreateInfo.imageExtent.height = app->windowHeight;
    swapchainCreateInfo.imageExtent.width = app->windowWidth;
    swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    swapchainCreateInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    swapchainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    swapchainCreateInfo.pNext = NULL;

    // ---- This is a critical section, only one thread should ever be in this section as a race condition may occur ----
    swapchains.push_front(VkSwapchainKHR{});
    appSwapchain.setRef(swapchains.begin());
    // ---- End of critical section


    THROW(vkCreateSwapchainKHR(app->logicalDevice.get(), &swapchainCreateInfo, NULL, &swapchains.front()), "Failed to create swapchain");
    uint32_t swapchainImageCount = 0;

    // Store all swapchain images
    THROW(vkGetSwapchainImagesKHR(app->logicalDevice.get(), swapchains.front(), &swapchainImageCount, NULL), "Failed to retrieve swapchain images");
    std::vector<VkImage> swapchainImages(swapchainImageCount);

    vkGetSwapchainImagesKHR(app->logicalDevice.get(), appSwapchain.get(), &swapchainImageCount, swapchainImages.data());

    for (VkImage image : swapchainImages) {
        AppImage swapchainImage = addExistingImage(app->windowWidth, app->windowHeight, AppImageTemplate::SWAPCHAIN_FORMAT, 1U, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL, image);
        appSwapchain.swapchainImages.push_back(swapchainImage);
        appSwapchain.swapchainImageViews.push_back(createImageView(appSwapchain.swapchainImages.back()));
    }

    app->maxFramesInFlight = swapchainImageCount;

    return appSwapchain;
}

void ResourceManager::destroySwapchain(AppSwapchain swapchain)
{
    // Destroy the created image views, and delete the images (which will themselves be destroyed when destroying the swapchain)
    for (uint32_t index = 0 ; index < swapchain.swapchainImages.size() ; index++){
        vkDestroyImageView(app->logicalDevice.get(), swapchain.swapchainImageViews[index].get(), nullptr);
        imageViews.erase(swapchain.swapchainImageViews[index].getRef());
        images.erase(swapchain.swapchainImages[index].getRef());

        vkDestroyFramebuffer(app->logicalDevice.get(), swapchain.framebuffers[index].get(), nullptr);
        frameBuffers.erase(swapchain.framebuffers[index].getRef());

    }

    // Destroy the swapchain Vulkan resource
    vkDestroySwapchainKHR(app->logicalDevice.get(), swapchain.get(), nullptr);

    // Remove this swapchain from the list
    swapchains.erase(swapchain.getRef());
}

AppFramebuffer ResourceManager::createFramebuffer(VkRenderPass renderPass, std::vector<VkImageView> attachmentViews)
{
    AppFramebuffer returnFramebuffer{};
    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = attachmentViews.size();
    createInfo.pAttachments = attachmentViews.data();
    createInfo.width = app->windowWidth;
    createInfo.height = app->windowHeight;
    createInfo.layers = 1;
    
    frameBuffers.push_front(VkFramebuffer{});
    returnFramebuffer.setRef(frameBuffers.begin());

    THROW(vkCreateFramebuffer(app->logicalDevice.get(), &createInfo, NULL, &frameBuffers.front()), "Failed to create framebuffer");

    return returnFramebuffer;
}

AppSampler ResourceManager::createSampler(AppSamplerTemplate t)
{
    AppSampler returnSampler;

    VkSamplerCreateInfo createInfo{getSamplerCreateInfoFromTemplate(t)};

    samplers.push_front({});
    returnSampler.setRef(samplers.begin());

    THROW(vkCreateSampler(app->logicalDevice.get(), &createInfo, nullptr, &samplers.front()), "Failed to create sampler");

    return returnSampler;
}

AppCommandPool ResourceManager::createCommandPool(uint32_t queueFamilyIndex)
{
    AppCommandPool commandPool{};
    commandPool.queueFamilyIndex = queueFamilyIndex;

    VkCommandPoolCreateInfo createInfo{};
    createInfo.pNext = nullptr;
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    createInfo.queueFamilyIndex = queueFamilyIndex;

    commandPools.push_front({});
    commandPool.setRef(commandPools.begin());

    THROW(vkCreateCommandPool(app->logicalDevice.get(), &createInfo, nullptr, &commandPools.front()), "Failed to create command pool");
    
    return commandPool;
}

VkCommandBuffer ResourceManager::allocateCommandBuffer(AppCommandPool pool, VkCommandBufferLevel level)
{
    VkCommandBuffer buffer;
    VkCommandBufferAllocateInfo allocInfo{};

    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.level = level;
    allocInfo.commandPool = pool.get();
    allocInfo.commandBufferCount = 1U;

   THROW(vkAllocateCommandBuffers(app->logicalDevice.get(), &allocInfo, &buffer), "Failed to allocate command buffer");

   return buffer;
}

void ResourceManager::destroy()
{
    destroySwapchain(app->swapchain);

    for (VkSampler sampler : samplers)
        vkDestroySampler(app->logicalDevice.get(), sampler, nullptr);

    for (VkBuffer buffer : buffers)
        vkDestroyBuffer(app->logicalDevice.get(), buffer, nullptr);

    // Destroy the image views first
    for (VkImageView imageView : imageViews) 
        vkDestroyImageView(app->logicalDevice.get(), imageView, nullptr);

    // Destroy images next
    for (VkImage image : images) 
        vkDestroyImage(app->logicalDevice.get(), image, nullptr);

    // Finally, destroy image memory
    for (VkDeviceMemory deviceMemory : deviceMemorySet) 
        vkFreeMemory(app->logicalDevice.get(), deviceMemory, nullptr);

    for (VkDescriptorPool descriptorPool : descriptorPools)
        vkDestroyDescriptorPool(app->logicalDevice.get(), descriptorPool, nullptr);

    for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayouts)
        vkDestroyDescriptorSetLayout(app->logicalDevice.get(), descriptorSetLayout, nullptr);

    for (VkRenderPass renderPass : renderPasses)
        vkDestroyRenderPass(app->logicalDevice.get(), renderPass, nullptr);
    
    for (VkShaderModule shaderModule : shaderModules)
        vkDestroyShaderModule(app->logicalDevice.get(), shaderModule, nullptr);

    for (VkPipeline pipeline : pipelines)
        vkDestroyPipeline(app->logicalDevice.get(), pipeline, nullptr);

    for (VkPipelineLayout pipelineLayout : pipelineLayouts)
        vkDestroyPipelineLayout(app->logicalDevice.get(), pipelineLayout, nullptr);

    for (VkFence fence : fences)
        vkDestroyFence(app->logicalDevice.get(), fence, nullptr);
    
    for (VkSemaphore semaphore : semaphores)
        vkDestroySemaphore(app->logicalDevice.get(), semaphore, nullptr);
    
    for (VkCommandPool commandPool : commandPools)
        vkDestroyCommandPool(app->logicalDevice.get(), commandPool, nullptr);

    for (VkSurfaceKHR surface : surfaces)
        vkDestroySurfaceKHR(app->instance.get(), surface, nullptr);
    
    for (VkDevice device : devices)
        vkDestroyDevice(device, nullptr);

    for (VkInstance instance : instances)
        vkDestroyInstance(instance, nullptr);
}

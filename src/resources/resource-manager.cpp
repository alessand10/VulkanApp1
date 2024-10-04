#include "resource-manager.h"
#include "GLFW/glfw3.h"

AppImage ResourceManager::createImage()
{
    AppImage returnImage{};
    images.push_front({});
    returnImage.setRef(images.begin());
    return returnImage;
}

void ResourceManager::destroyImage(AppImage image, VkDevice device)
{
    vkDestroyImage(device, image.get(), nullptr);
    images.erase(image.getIterator());
}

AppImageView ResourceManager::createImageView()
{
    AppImageView returnImageView{};
    imageViews.push_front({});
    returnImageView.setRef(imageViews.begin());
    return returnImageView;
}

void ResourceManager::destroyImageView(AppImageView imageView, VkDevice device)
{
    vkDestroyImageView(device, imageView.get(), nullptr);
    imageViews.erase(imageView.getIterator());
}

void ResourceManager::destroy()
{
    // destroySwapchain(app->swapchain);

    // for (VkSampler sampler : samplers)
    //     vkDestroySampler(app->logicalDevice.get(), sampler, nullptr);

    // for (VkBuffer buffer : buffers)
    //     vkDestroyBuffer(app->logicalDevice.get(), buffer, nullptr);

    // // Destroy the image views first
    // for (VkImageView imageView : imageViews) 
    //     vkDestroyImageView(app->logicalDevice.get(), imageView, nullptr);

    // // Destroy images next
    // for (VkImage image : images) 
    //     vkDestroyImage(app->logicalDevice.get(), image, nullptr);

    // // Finally, destroy image memory
    // for (VkDeviceMemory deviceMemory : deviceMemorySet) 
    //     vkFreeMemory(app->logicalDevice.get(), deviceMemory, nullptr);

    // for (VkDescriptorPool descriptorPool : descriptorPools)
    //     vkDestroyDescriptorPool(app->logicalDevice.get(), descriptorPool, nullptr);

    // for (VkDescriptorSetLayout descriptorSetLayout : descriptorSetLayouts)
    //     vkDestroyDescriptorSetLayout(app->logicalDevice.get(), descriptorSetLayout, nullptr);

    // for (VkRenderPass renderPass : renderPasses)
    //     vkDestroyRenderPass(app->logicalDevice.get(), renderPass, nullptr);
    
    // for (VkShaderModule shaderModule : shaderModules)
    //     vkDestroyShaderModule(app->logicalDevice.get(), shaderModule, nullptr);

    // for (VkPipeline pipeline : pipelines)
    //     vkDestroyPipeline(app->logicalDevice.get(), pipeline, nullptr);

    // for (VkPipelineLayout pipelineLayout : pipelineLayouts)
    //     vkDestroyPipelineLayout(app->logicalDevice.get(), pipelineLayout, nullptr);

    // for (VkFence fence : fences)
    //     vkDestroyFence(app->logicalDevice.get(), fence, nullptr);
    
    // for (VkSemaphore semaphore : semaphores)
    //     vkDestroySemaphore(app->logicalDevice.get(), semaphore, nullptr);
    
    // for (VkCommandPool commandPool : commandPools)
    //     vkDestroyCommandPool(app->logicalDevice.get(), commandPool, nullptr);

    // for (VkSurfaceKHR surface : surfaces)
    //     vkDestroySurfaceKHR(app->instance.get(), surface, nullptr);
    
    // for (VkDevice device : devices)
    //     vkDestroyDevice(device, nullptr);

    // for (VkInstance instance : instances)
    //     vkDestroyInstance(instance, nullptr);
}

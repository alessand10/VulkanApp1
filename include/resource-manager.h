#pragma once
#include "vulkan/vulkan.hpp"
#include "resource-structs.h"
#include <map>
#include <list>

/**
 * @class ResourceManager
 * @brief Manages the creation and destruction of Vulkan resources.
 * 
 * The resource manager class provides an extensive list of operations to create and destroy
 * application resources.
 */

class ResourceManager {
    private:
    
    public:
    class VulkanApp* app;

    std::list<VkInstance> instances;
    AppInstance createInstance(const char* appName, bool enableValidationLayers);

    std::list<VkSurfaceKHR> surfaces;
    AppSurface createSurface(AppInstance instance, class GLFWwindow* window);

    std::list<VkDevice> devices;
    AppDevice createDevice(VkPhysicalDevice physicalDevice, QueueFamilyIndices* queueFamilyIndices);

    std::list<VkDeviceMemory> deviceMemorySet;

    std::list<VkImage> images;
    std::list<VkImageView> imageViews;

    void init(class VulkanApp* appRef);
    uint32_t getSuitableMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags propertyFlags);

    /**
     * @brief Creates an app-managed image using one of the defined templates
     * 
     * @note The created image is automatically destroyed when the app terminates.
     * 
     * @param width The width of the image
     * @param height The height of the image
     * @param appImageTemplate The template to create the image from
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    AppImage createImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers = 1U);

    AppImage addExistingImage(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layers, VkImageLayout layout, VkImage image);

    void destroyImage(AppImage image);

    /**
     * @brief Creates an app-managed image view based on the image & template passed in
     * 
     * @note The created image view is automatically destroyed when the app terminates.
     * @note The created view is returned via the image structure argument.
     * 
     * @param image The app image object to create a view for
     * @param layerCount The number of array layers that this image view will encompass
     * @param baseLayer The starting array layer that this image view will encompass
     */
    AppImageView createImageView(AppImage &image, uint32_t layerCount = 1U, uint32_t baseLayer = 0U);
    void destroyImageView(AppImageView &imageView);

    /**
     * @brief Allocates memory for an app image
     * 
     * @note The allocated image memory is automatically freed when the app terminates.
     * @note Memory allocation flags are set based on the image creation template.

     * @param image The app image object to allocate memory for
     */
    AppDeviceMemory allocateImageMemory(AppImage &image);
    void bindImageToMemory(AppImage &image, AppDeviceMemory &imageMemory);
    void destroyDeviceMemory(AppDeviceMemory deviceMemory);


    /**
     * @brief Creates an app-managed image, image memory, and image view. The image is also bound to its memory.
     * 
     * @note The created image, image view and memory are destroyed/freed automatically
     * 
     * @param width The width of the image
     * @param height The height of the image
     * @param appImageTemplate The template to create the image from
     * @param layerCount The number of array layers to create for this image
     * 
     * @return The created image bundle (containing an image, view and memory)
     */
    AppImageBundle createImageAll(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount = 1U);

    /**
     * @brief Transitions an image from its current layout to a new layout
     * 
     * The application must have a command buffer to transition an image's layout.
     * 
     * @param image The image to transition the layout of
     * @param newLayout The layout to transition the new image to
     * @param targetLayer The layer to transition the layout of
     */
    void transitionImageLayout(AppImage &image, VkImageLayout newLayout, VkCommandBuffer commandBuffer, uint32_t targetLayer = 0U, uint32_t layerCount = 1U);

    /**
     * @brief Pushes the content of a staging image (with a linear layout) to a device-local image.
     * 
     * @note The returned image will be in the layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, it can be
     * transitioned for further use with the transitionImageLayout() method.
     * 
     * @param stagingImage The staging image to copy data from
     * @param deviceLocalImage The device-local image to copy data to
     */
    void pushStagingImage(AppImage &stagingImage, AppImage &deviceLocalImage, VkCommandBuffer commandBuffer, uint32_t deviceLocalLayer = 0);

    std::list<VkBuffer> buffers = {};

    AppBuffer createBuffer(AppBufferTemplate bufferTemplate, size_t size);
    AppDeviceMemory allocateBufferMemory(AppBuffer &appBuffer);
    void bindBufferToMemory(AppBuffer &appBuffer, AppDeviceMemory &deviceMemory);
    AppBufferBundle createBufferAll(AppBufferTemplate bufferTemplate, size_t size);
    void pushStagingBuffer(AppBuffer &stagingBuffer, AppBuffer &deviceLocalBuffer, VkCommandBuffer commandBuffer);
    void copyDataToStagingMemory(AppDeviceMemory &stagingMemory, void* memory, size_t size);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDescriptorPool> descriptorPools;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<AppDescriptorItemTemplate> descriptorItems);

    /**
     * @brief Creates a descriptor pool capable of allocated the specified number of descriptor sets/types
     * 
     * @param maxSetsCount The total number of descriptors sets that can be allocated from this pool
     * @param descriptorTypeCounts A map specifying the total number of each descriptor type that can be allocated across all descriptor sets
     */
    VkDescriptorPool createDescriptorPool(uint32_t maxSetsCount, std::map<AppDescriptorItemTemplate, uint32_t> descriptorTypeCounts);

    /**
     * @brief Allocates a single descriptor set of the provided layout from the specified descriptor pool
     * 
     * @param descriptorSetLayout The descriptor set layout used to create the descriptor set
     * @param descriptorPool The pool from which the descriptor set is allocated
     */
    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);


    /**
     * @brief Updates an image descriptor for a particular descriptor set
     */
    void updateImageDescriptor(AppImageView imageView, VkDescriptorSet set, uint32_t binding, AppDescriptorItemTemplate itemTemplate, AppSampler sampler = AppSampler{});
    void updateBufferDescriptor(VkDescriptorSet set, VkBuffer buffer, uint32_t size,  uint32_t binding, AppDescriptorItemTemplate itemTemplate);


    std::vector<VkRenderPass> renderPasses;

    /**
     * @brief Creates an app-managed render pass 
     * 
     * @param attachments A vector of attachments that will be used throughout all subpasses. Each attachment is of the form {VkImageView imageView, AppAttachmentTemplate}
     * @param subpasses A vector of subpasses for this render pass. Each subpass consists of a vector of attachments references for the subpass: { subpass 1: [{AttachmentIndex, Image Layout},{Attachment Index, Image Layout}], subpass 2: ... }
     */
    VkRenderPass createRenderPass(std::vector<AppAttachment> attachments, std::vector<AppSubpass> subpasses, std::vector<VkSubpassDependency> subpassDependencies);

    std::vector<VkShaderModule> shaderModules;

    AppShaderModule createShaderModule(std::string path, VkShaderStageFlagBits shaderStageFlags);

    std::vector<VkPipeline> pipelines;
    std::vector<VkPipelineLayout> pipelineLayouts;

    /**
     * @brief Create a Vulkan Pipeline Layout object
     * 
     * @param descriptorSetLayouts The descriptor set layout to create the pipeline layout with
     * @param pushConstantRanges The push constant ranges to create the pipeline layout with. Structure: {VkShaderStageFlags stageFlags; uint32_t offset; uint32_t size;}
     * @return VkPipelineLayout 
     */
    VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts, std::vector<VkPushConstantRange> pushConstantRanges = {});
    VkPipeline createGraphicsPipeline(std::vector<AppShaderModule> shaderModules, VkPipelineLayout pipelineLayout, VkRenderPass renderPass);

    std::vector<VkFence> fences;
    std::vector<VkSemaphore> semaphores;

    VkFence createFence(bool signaled);
    VkSemaphore createSemaphore();

    std::list<VkSwapchainKHR> swapchains;

    AppSwapchain createSwapchain();
    void destroySwapchain(AppSwapchain swapchain);

    std::list<VkFramebuffer> frameBuffers;

    AppFramebuffer createFramebuffer(VkRenderPass renderPass, std::vector<VkImageView> attachmentViews);

    std::list<VkSampler> samplers;

    AppSampler createSampler(AppSamplerTemplate t);

    std::list<VkCommandPool> commandPools;

    AppCommandPool createCommandPool(uint32_t queueFamilyIndex);
    VkCommandBuffer allocateCommandBuffer(AppCommandPool pool, VkCommandBufferLevel level);

    void destroy();
};
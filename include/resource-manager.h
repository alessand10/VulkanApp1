#pragma once
#include "vulkan/vulkan.hpp"
#include "resource-structs.h"
#include <map>
#include <list>

/**
 * @class ResourceManager
 * @brief Manages the creation and destruction of Vulkan resources.
 */

class ResourceManager {
    private:
    
    public:
    class VulkanApp* app;

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
     * @param layers The number of array layers to create for this image
     * 
     * @return The created image object
     */
    AppImageBundle createImageAll(uint32_t width, uint32_t height, AppImageTemplate appImageTemplate, uint32_t layerCount = 1U);

    /**
     * @brief Transitions an image from its current layout to a new layout
     * 
     * @param image The image to transition the layout of
     * @param newLayout The layout to transition the new image to
     */
    void transitionImageLayout(AppImage &image, VkImageLayout newLayout);

    /**
     * @brief Create an app-managed image view based on the image & template passed in
     * 
     * @note The returned image will be in the layout VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, it can be
     * transitioned for further use with the transitionImageLayout() method.
     * 
     * @param stagingImage The staging image to copy data from
     * @param deviceLocalImage The device-local image to copy data to
     */
    void pushStagingImage(AppImage &stagingImage, AppImage &deviceLocalImage);

    std::list<VkBuffer> buffers = {};

    AppBuffer createBuffer(AppBufferTemplate bufferTemplate, size_t size);
    AppDeviceMemory allocateBufferMemory(AppBuffer &appBuffer);
    void bindBufferToMemory(AppBuffer &appBuffer, AppDeviceMemory &deviceMemory);
    AppBufferBundle createBufferAll(AppBufferTemplate bufferTemplate, size_t size);
    void pushStagingBuffer(AppBuffer &stagingBuffer, AppBuffer &deviceLocalBuffer);
    void copyDataToStagingBuffer(AppDeviceMemory &stagingBufferMemory, void* memory, size_t size);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
    std::vector<VkDescriptorPool> descriptorPools;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorSetLayout createDescriptorSetLayout(std::vector<AppDescriptorItemTemplate> descriptorItems);
    VkDescriptorPool createDescriptorPool(uint32_t maxSetsCount, std::map<AppDescriptorItemTemplate, uint32_t> descriptorTypeCounts);
    VkDescriptorSet allocateDescriptorSet(VkDescriptorSetLayout descriptorSetLayout, VkDescriptorPool descriptorPool);
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

    VkPipelineLayout createPipelineLayout(std::vector<VkDescriptorSetLayout> descriptorSetLayouts);
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
#pragma once
#include "app-resource.h"
#include "image-resource.h"

class AppImageView : public AppResource<VkImageView> {
    AppImageTemplate imageCreationTemplate;
    public:
    AppImageTemplate getTemplate() { return imageCreationTemplate; }

    void init(class VulkanApp* app, AppImage &image, uint32_t layerCount, uint32_t baseLayer);
    void init(class VulkanApp* app, VkImage image, AppImageTemplate imageCreationTemplate, uint32_t layerCount, uint32_t baseLayer);
    
    void destroy();
};
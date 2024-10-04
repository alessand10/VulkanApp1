#pragma once
#include "app-resource.h"
#include "image-resource.h"

class AppImageView : public AppResource<VkImageView> {
    AppImageTemplate imageCreationTemplate;
    public:
    AppImageTemplate getTemplate() { return imageCreationTemplate; }
    void init(VulkanApp* app, AppImage &image, uint32_t layerCount, uint32_t baseLayer);
    
    void destroy() { getApp()->resources.imageViews.destroy(getIterator(), getApp()->logicalDevice.get()); }
};
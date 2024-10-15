#pragma once
#include "app-resource.h"
#include "image-resource.h"

class AppImageView : public AppResource<VkImageView> {
    AppImageTemplate imageCreationTemplate;
    public:
    AppImageTemplate getTemplate() { return imageCreationTemplate; }

    void init(class AppBase* appBase, AppImage &image, uint32_t layerCount, uint32_t baseLayer);
    void init(class AppBase* appBase, VkImage image, AppImageTemplate imageCreationTemplate, uint32_t layerCount, uint32_t baseLayer);
    
    void destroy();
};
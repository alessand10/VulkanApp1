#pragma once
#include "app-resource.h"

class AppFramebuffer : public AppResource<VkFramebuffer> {
    public:
    void init(class VulkanApp* app, VkRenderPass renderPass, std::vector<VkImageView> attachmentViews);
    
    void destroy();
};
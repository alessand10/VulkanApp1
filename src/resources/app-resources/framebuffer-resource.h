#pragma once
#include "app-resource.h"

class AppFramebuffer : public AppResource<VkFramebuffer> {
    public:
    void init(VulkanApp* app, VkRenderPass renderPass, std::vector<VkImageView> attachmentViews);
    
    void destroy() { getApp()->resources.framebuffers.destroy(getIterator(), getApp()->logicalDevice.get()); }
};
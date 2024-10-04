#pragma once
#include "app-resource.h"

enum class AttachmentType {
    COLOR,
    DEPTH_STENCIL
};

class AppRenderPass : public AppResource<VkRenderPass> {
    public:
    void init(VulkanApp* app, std::vector<AppAttachment> attachments, std::vector<AppSubpass> subpasses, std::vector<VkSubpassDependency> subpassDependencies);
    void destroy() { getApp()->resources.renderPasses.destroy(getIterator(), getApp()->logicalDevice.get()); }
};
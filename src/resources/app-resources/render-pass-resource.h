#pragma once
#include "app-resource.h"

enum class AttachmentType {
    COLOR,
    DEPTH_STENCIL
};

enum class AttachmentTemplate {
    SWAPCHAIN_COLOR_ATTACHMENT,
    SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT,
    APP_TEXTURE_COLOR_ATTACHMENT,
};

struct AppSubpassAttachmentRef {
    uint32_t attachmentIndex;
    VkImageLayout imageLayout;
};

struct AppSubpass {
    std::vector<AppSubpassAttachmentRef> attachmentRefs;
};

class AppRenderPass : public AppResource<VkRenderPass> {
    public:
    void init(class AppBase* appBase, std::vector<AttachmentTemplate> attachments, std::vector<AppSubpass> subpasses, std::vector<VkSubpassDependency> subpassDependencies);
    void destroy();
};
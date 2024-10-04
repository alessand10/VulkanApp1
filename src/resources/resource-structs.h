#pragma once

#include <vulkan/vulkan.hpp>
#include <list>
#include "device-memory-resource.h"


struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};



struct AppImageBundle {
    AppImage image;
    AppImageView imageView;
    AppDeviceMemory deviceMemory;
};


struct AppBufferBundle {
    AppBuffer buffer;
    AppDeviceMemory deviceMemory;
};

class AppFramebuffer : public AppResource<VkFramebuffer> {
    public:
    AppFramebuffer(VulkanApp* app, std::list<VkFramebuffer>::iterator resourceIt) : AppResource(app, resourceIt) {}
};

enum class AppDescriptorItemTemplate {
    VS_UNIFORM_BUFFER,
    CS_UNIFORM_BUFFER,
    FS_SAMPLED_IMAGE_WITH_SAMPLER,
    CS_STORAGE_IMAGE,
};

struct AppShaderModule {
    VkShaderModule shaderModule;
    VkShaderStageFlagBits shaderStage;
};

enum class AppAttachmentTemplate {
    SWAPCHAIN_COLOR_ATTACHMENT,
    SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT,
    APP_TEXTURE_COLOR_ATTACHMENT,
};

struct AppAttachment {
    AppAttachmentTemplate attachmentTemplate;
};


struct AppSubpassAttachmentRef {
    uint32_t attachmentIndex;
    VkImageLayout imageLayout;
};

struct AppSubpass {
    std::vector<AppSubpassAttachmentRef> attachmentRefs;
};

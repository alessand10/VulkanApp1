#pragma once

#include <vulkan/vulkan.hpp>
#include <list>
#include "app-resource.h"


enum class AppImageTemplate {
    PREWRITTEN_SAMPLED_TEXTURE = 0U,
    STAGING_IMAGE_TEXTURE = 1U,
    DEVICE_WRITE_SAMPLED_TEXTURE = 2U,
    DEPTH_STENCIL,
    SWAPCHAIN_FORMAT
};

enum class AppBufferTemplate {
    UNIFORM_BUFFER,
    VERTEX_BUFFER_DEVICE,
    INDEX_BUFFER_DEVICE,
    VERTEX_BUFFER_STAGING,
    INDEX_BUFFER_STAGING
};

struct AppInstance : AppResource<VkInstance>{};
struct AppSurface : AppResource<VkSurfaceKHR>{};
struct AppDevice : AppResource<VkDevice> {};

struct QueueFamilyIndices {
    uint32_t graphics;
    uint32_t compute;
    uint32_t transfer;
};

/**
 * A Wrapper around a VkImage, VkDeviceMemory, VkImageView, and VkSampler which holds information regarding the image
 */
struct AppImage : AppResource<VkImage> {
    AppImageTemplate imageCreationTemplate = AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE;
    uint32_t layerCount = 1U;
    uint32_t width;
    uint32_t height;
    VkImageLayout imageLayout;
};

struct AppImageView : AppResource<VkImageView> {
    AppImageTemplate imageCreationTemplate = AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE;
};

struct AppDeviceMemory : AppResource<VkDeviceMemory> {

};

struct AppCommandPool : AppResource<VkCommandPool>{
    uint32_t queueFamilyIndex;
};

struct AppImageBundle {
    AppImage image;
    AppImageView imageView;
    AppDeviceMemory deviceMemory;
};

struct AppBuffer : AppResource<VkBuffer> {
    AppBufferTemplate appBufferTemplate = AppBufferTemplate::UNIFORM_BUFFER;
    size_t size;
};

struct AppBufferBundle {
    AppBuffer buffer;
    AppDeviceMemory deviceMemory;
};

struct AppFramebuffer : AppResource<VkFramebuffer> {

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

struct AppSwapchain : AppResource<VkSwapchainKHR> {
    std::vector<AppImage> swapchainImages;
    std::vector<AppImageView> swapchainImageViews;
    std::vector<AppFramebuffer> framebuffers;
};

struct AppAttachment {
    AppAttachmentTemplate attachmentTemplate;
};

enum class AppSamplerTemplate {
    DEFAULT
};

struct AppSampler : AppResource<VkSampler> {

};

static VkSamplerCreateInfo getSamplerCreateInfoFromTemplate(AppSamplerTemplate t) {
    switch (t) {
        case AppSamplerTemplate::DEFAULT : {
            return {
                VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
                nullptr,
                0U,
                VK_FILTER_LINEAR,
                VK_FILTER_LINEAR,
                VK_SAMPLER_MIPMAP_MODE_LINEAR,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                VK_SAMPLER_ADDRESS_MODE_REPEAT,
                0.f,
                VK_FALSE,
                16.0f,
                VK_TRUE,
                VK_COMPARE_OP_ALWAYS,
                0.f,
                1.f,
                VK_BORDER_COLOR_INT_OPAQUE_BLACK,
                VK_FALSE,
            };
        }
        default: 
            return {};
    }
};

struct AppSubpassAttachmentRef {
    uint32_t attachmentIndex;
    VkImageLayout imageLayout;
};

struct AppSubpass {
    std::vector<AppSubpassAttachmentRef> attachmentRefs;
};

enum class AttachmentType {
    COLOR,
    DEPTH_STENCIL
};

static AttachmentType getAttachmentTypeFromTemplate(AppAttachmentTemplate t) {
    switch (t) {
        case AppAttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT: 
        case AppAttachmentTemplate::APP_TEXTURE_COLOR_ATTACHMENT: 
            return AttachmentType::COLOR;
        case AppAttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT:
            return AttachmentType::DEPTH_STENCIL;
        default: return AttachmentType::COLOR;
    }
}

static VkAttachmentDescription getAttachmentDescriptionFromTemplate(AppAttachmentTemplate t) {
    /**
     * flags;
     * format;
     * samples;
     * loadOp;
     * storeOp;
     * stencilLoadOp;
     * stencilStoreOp;
     * initialLayout;
     * finalLayout;
     */
    switch(t) {
        case AppAttachmentTemplate::SWAPCHAIN_COLOR_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_B8G8R8A8_SRGB,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
            };
        }
        case AppAttachmentTemplate::SWAPCHAIN_DEPTH_STENCIL_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_D32_SFLOAT_S8_UINT,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL
            };
        }
        case AppAttachmentTemplate::APP_TEXTURE_COLOR_ATTACHMENT : {
            return {
                0U,
                VK_FORMAT_R8G8B8A8_UNORM,
                VK_SAMPLE_COUNT_1_BIT,
                VK_ATTACHMENT_LOAD_OP_CLEAR,
                VK_ATTACHMENT_STORE_OP_STORE,
                VK_ATTACHMENT_LOAD_OP_DONT_CARE,
                VK_ATTACHMENT_STORE_OP_DONT_CARE,
                VK_IMAGE_LAYOUT_UNDEFINED,
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
            };
        }
        default : 
        return {};
    }
}

struct AppDescriptor {
    AppDescriptorItemTemplate descriptorTemplate;
    uint32_t binding;
};

struct AppImageDescriptor : AppDescriptor {
    AppImage appImage;
    VkSampler sampler = VK_NULL_HANDLE;
};

struct AppBufferDescriptor : AppDescriptor {

};

static VkBufferCreateInfo getBufferCreateInfoFromTemplate(AppBufferTemplate t) { 
    switch(t) {
        case AppBufferTemplate::UNIFORM_BUFFER :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           0U,
           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           0U,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::VERTEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           0U,
           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_STAGING :
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           0U,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        case AppBufferTemplate::INDEX_BUFFER_DEVICE : 
        return {
           VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
           nullptr,
           0U,
           0U,
           VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
           VK_SHARING_MODE_EXCLUSIVE,
           0U,
           nullptr
        };
        default:
            return {};
    }
};

static VkImageCreateInfo getImageCreateInfoFromTemplate(AppImageTemplate t) { 
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {0U, 0U, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            1U, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::STAGING_IMAGE_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {0U, 0U, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            1U, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_LINEAR, //tiling
            VK_IMAGE_USAGE_TRANSFER_SRC_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            {0U, 0U, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            1U, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        case AppImageTemplate::DEPTH_STENCIL : 
        return {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO, //sType
            NULL, //pNext
            0U, //flags
            VK_IMAGE_TYPE_2D, //imageType
            VK_FORMAT_D32_SFLOAT_S8_UINT, //format
            {0U, 0U, 1U}, // extent {width, height, depth}
            1U, // mipLevels
            1U, // arrayLayers
            VK_SAMPLE_COUNT_1_BIT, //samples
            VK_IMAGE_TILING_OPTIMAL, //tiling
            VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, //usage
            VK_SHARING_MODE_EXCLUSIVE, //sharingMode
            0U, //queueFamilyIndexCount
            nullptr, // pQueueFamilyIndices
            VK_IMAGE_LAYOUT_UNDEFINED // initialLayout
        };
        default: 
        return {};
    };
};


static VkImageViewCreateInfo getImageViewCreateInfoFromTemplate(AppImageTemplate t) {
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE:
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE:
        case AppImageTemplate::STAGING_IMAGE_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            VK_NULL_HANDLE, //image
            VK_IMAGE_VIEW_TYPE_2D, //viewType
            VK_FORMAT_R8G8B8A8_UNORM, //format
            { 
                VK_COMPONENT_SWIZZLE_IDENTITY, //r
                VK_COMPONENT_SWIZZLE_IDENTITY, //g
                VK_COMPONENT_SWIZZLE_IDENTITY, //b
                VK_COMPONENT_SWIZZLE_IDENTITY //a
            },
            { 
                VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
                0U, //baseMipLevel
                1U, //levelCount
                0U, //baseArrayLayer
                1U //layerCount
            }
        };

        case AppImageTemplate::DEPTH_STENCIL :
        return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            VK_NULL_HANDLE, //image
            VK_IMAGE_VIEW_TYPE_2D, //viewType
            VK_FORMAT_D32_SFLOAT_S8_UINT, //format
            { 
                VK_COMPONENT_SWIZZLE_IDENTITY, //r
                VK_COMPONENT_SWIZZLE_IDENTITY, //g
                VK_COMPONENT_SWIZZLE_IDENTITY, //b
                VK_COMPONENT_SWIZZLE_IDENTITY //a
            },
            { 
                VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, //aspectMask
                0U, //baseMipLevel
                1U, //levelCount
                0U, //baseArrayLayer
                1U //layerCount
            }
        };
        case AppImageTemplate::SWAPCHAIN_FORMAT : {
            return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            VK_NULL_HANDLE, //image
            VK_IMAGE_VIEW_TYPE_2D, //viewType
            VK_FORMAT_B8G8R8A8_SRGB, //format
            { 
                VK_COMPONENT_SWIZZLE_IDENTITY, //r
                VK_COMPONENT_SWIZZLE_IDENTITY, //g
                VK_COMPONENT_SWIZZLE_IDENTITY, //b
                VK_COMPONENT_SWIZZLE_IDENTITY //a
            },
            { 
                VK_IMAGE_ASPECT_COLOR_BIT, //aspectMask
                0U, //baseMipLevel
                1U, //levelCount
                0U, //baseArrayLayer
                1U //layerCount
            }
        };
        }

        default:
            return {};
    };
};

static VkImageLayout getImageLayoutFromTemplate(AppImageTemplate t) {
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE : {
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }
        case AppImageTemplate::STAGING_IMAGE_TEXTURE : {
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        }
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE : {
            return VK_IMAGE_LAYOUT_GENERAL;
        }
        default:
            return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
}

static VkDescriptorType getDescriptorTypeFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        }
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE : {
            return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        }
        default:
            return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    }
}

static VkShaderStageFlags getDescriptorStageFlagFromTemplate(AppDescriptorItemTemplate t) {
    switch(t) {
        case AppDescriptorItemTemplate::VS_UNIFORM_BUFFER : {
            return VK_SHADER_STAGE_VERTEX_BIT;
        }
        case AppDescriptorItemTemplate::FS_SAMPLED_IMAGE_WITH_SAMPLER : {
            return VK_SHADER_STAGE_FRAGMENT_BIT;
        }
        case AppDescriptorItemTemplate::CS_UNIFORM_BUFFER:
        case AppDescriptorItemTemplate::CS_STORAGE_IMAGE:
            return VK_SHADER_STAGE_COMPUTE_BIT;
        default:
            return VK_SHADER_STAGE_VERTEX_BIT;
    }
}
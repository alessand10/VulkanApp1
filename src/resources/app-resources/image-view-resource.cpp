#include "app-base.h"
#include "image-view-resource.h"

static VkImageViewCreateInfo getImageViewCreateInfoFromTemplate(AppImageTemplate t, VkImage image, uint32_t layerCount, uint32_t baseLayer) {
    switch(t) {
        case AppImageTemplate::PREWRITTEN_SAMPLED_TEXTURE:
        case AppImageTemplate::DEVICE_WRITE_SAMPLED_TEXTURE:
        case AppImageTemplate::STAGING_IMAGE_TEXTURE:
        return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            image, //image
            layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, //viewType
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
                baseLayer, //baseArrayLayer
                layerCount //layerCount
            }
        };

        case AppImageTemplate::DEPTH_STENCIL :
        return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            image, //image
            layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, //viewType
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
                baseLayer, //baseArrayLayer
                layerCount //layerCount
            }
        };
        case AppImageTemplate::SWAPCHAIN_FORMAT : {
            return {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO, // sType
            NULL, //pNext
            0U, //flags
            image, //image
            layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D, //viewType
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
                baseLayer, //baseArrayLayer
                layerCount //layerCount
            }
        };
        }

        default:
            return {};
    };
};

void AppImageView::init(AppBase* appBase, AppImage &image, uint32_t layerCount, uint32_t baseLayer)
{
    VkImageViewCreateInfo createInfo{ getImageViewCreateInfoFromTemplate(image.getTemplate(), image.get(), layerCount, baseLayer) };
    imageCreationTemplate = image.getTemplate();
    VkImageView imageView;
    THROW(vkCreateImageView(appBase->getDevice(), &createInfo, nullptr, &imageView), "Failed to create image view");
    
    AppResource::init(appBase, appBase->resources.imageViews.create(imageView));
}

void AppImageView::init(AppBase* appBase, VkImage image, AppImageTemplate imageCreationTemplate, uint32_t layerCount, uint32_t baseLayer)
{
    VkImageViewCreateInfo createInfo{ getImageViewCreateInfoFromTemplate(imageCreationTemplate, image, layerCount, baseLayer) };

    VkImageView imageView;
    THROW(vkCreateImageView(appBase->getDevice(), &createInfo, nullptr, &imageView), "Failed to create image view");
    
    AppResource::init(appBase, appBase->resources.imageViews.create(imageView));
}

void AppImageView::destroy()
{
    appBase->resources.imageViews.destroy(getIterator(), appBase->getDevice());
}

#include "framebuffer-resource.h"

void AppFramebuffer::init(VulkanApp *app, VkRenderPass renderPass, std::vector<VkImageView> attachmentViews)
{

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass;
    createInfo.attachmentCount = attachmentViews.size();
    createInfo.pAttachments = attachmentViews.data();
    createInfo.width = app->windowWidth;
    createInfo.height = app->windowHeight;
    createInfo.layers = 1;
    
    VkFramebuffer framebuffer;
    THROW(vkCreateFramebuffer(app->logicalDevice.get(), &createInfo, NULL, &framebuffer), "Failed to create framebuffer");

    AppResource::init(app, app->resources.framebuffers.create(framebuffer));
}

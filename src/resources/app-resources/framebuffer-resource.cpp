#include "app-base.h"
#include "framebuffer-resource.h"
#include "image-view-resource.h"
#include "render-pass-resource.h"

void AppFramebuffer::init(AppBase* appBase, AppRenderPass* renderPass, std::vector<AppImageView*> attachmentViews)
{   
    std::vector<VkImageView> imageViews = {};

    for (uint32_t index = 0U ; index < attachmentViews.size() ; index ++) {
        imageViews.push_back(attachmentViews[index]->get());
    }

    VkFramebufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    createInfo.renderPass = renderPass->get();
    createInfo.attachmentCount = imageViews.size();
    createInfo.pAttachments = imageViews.data();
    createInfo.width = appBase->viewportSettings.width;
    createInfo.height = appBase->viewportSettings.height;
    createInfo.layers = 1;
    
    VkFramebuffer framebuffer;
    THROW(vkCreateFramebuffer(appBase->getDevice(), &createInfo, NULL, &framebuffer), "Failed to create framebuffer");

    AppResource::init(appBase, appBase->resources.framebuffers.create(framebuffer));
}

void AppFramebuffer::destroy()
{
    appBase->resources.framebuffers.destroy(getIterator(), appBase->getDevice());
}

#pragma once
#include "app-resource.h"

class AppFramebuffer : public AppResource<VkFramebuffer> {
    public:
    void init(class AppBase* appBase, class AppRenderPass* renderPass, std::vector<class AppImageView*> attachmentViews);
    
    void destroy();
};
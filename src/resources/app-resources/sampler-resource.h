#pragma once
#include "app-resource.h"

enum class AppSamplerTemplate {
    DEFAULT
};

class AppSampler : public AppResource<VkSampler> {
    AppSamplerTemplate samplerTemplate;
    public:
    void init(VulkanApp* app, AppSamplerTemplate samplerTemplate);
    void destroy() { getApp()->resources.samplers.destroy(getIterator(), getApp()->logicalDevice.get()); }
};
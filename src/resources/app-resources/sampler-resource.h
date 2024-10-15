#pragma once
#include "app-resource.h"

enum class AppSamplerTemplate {
    DEFAULT
};

class AppSampler : public AppResource<VkSampler> {
    AppSamplerTemplate samplerTemplate;
    public:
    void init(class AppBase* appBase, AppSamplerTemplate samplerTemplate);
    void destroy();
};
#include "sampler-resource.h"

void AppSampler::init(VulkanApp *app, AppSamplerTemplate samplerTemplate)
{
    this->samplerTemplate = samplerTemplate;

    VkSamplerCreateInfo createInfo{getSamplerCreateInfoFromTemplate(samplerTemplate)};

    VkSampler sampler;
    THROW(vkCreateSampler(app->logicalDevice.get(), &createInfo, nullptr, &sampler), "Failed to create sampler");

    AppResource::init(app, app->resources.samplers.create(sampler));
}

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
#pragma once
#include "app-resource.h"

class AppDescriptorSetLayout : public AppResource<VkDescriptorSetLayout> {
    public:
    void init(VulkanApp* app, std::vector<AppDescriptorItemTemplate> descriptorItems);

    void destroy() { getApp()->resources.descriptorSetLayouts.destroy(getIterator(), getApp()->logicalDevice.get()); }  
};
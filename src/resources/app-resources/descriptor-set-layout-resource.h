#pragma once
#include "app-resource.h"
#include "descriptor-pool-resource.h"

struct DescriptorItem {
    VkDescriptorType descriptorType;
    VkShaderStageFlags shaderStage;
};

class AppDescriptorSetLayout : public AppResource<VkDescriptorSetLayout> {
    public:
    void init(class AppBase* appBase, std::vector<DescriptorItem> descriptorItems);

    void destroy();  
};
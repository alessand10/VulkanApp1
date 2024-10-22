#pragma once
#include "vulkan/vulkan.h"
#include "material-input.h"

// Bind this shader module and texture array
class Material {
    VkDescriptorSet descriptorSet;

    public:
    void setDescriptorSet(VkDescriptorSet descriptorSet) {
        this->descriptorSet = descriptorSet;
    }
    VkDescriptorSet getDescriptorSet() {
        return descriptorSet;
    }
};
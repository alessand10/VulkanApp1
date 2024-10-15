#pragma once
#include "app-resource.h"
#include <map>


enum class AppDescriptorItemTemplate {
    VS_UNIFORM_BUFFER,
    CS_UNIFORM_BUFFER,
    FS_SAMPLED_IMAGE_WITH_SAMPLER,
    CS_STORAGE_IMAGE,
};

class AppDescriptorPool : public AppResource<VkDescriptorPool> {
    uint32_t maxSetsCount;
    std::map<VkDescriptorType, uint32_t> descriptorTypeCounts;
    public:

    /**
     * @brief Creates a descriptor pool capable of allocated the specified number of descriptor sets/types
     * 
     * @param app The application object
     * @param maxSetsCount The total number of descriptors sets that can be allocated from this pool
     * @param descriptorTypeCounts A map specifying the total number of each descriptor type that can be allocated across all descriptor sets
     */
    void init(class AppBase* appBase, uint32_t maxSetsCount, std::map<VkDescriptorType, uint32_t> descriptorTypeCounts);
    
    /**
     * @brief Allocates a single descriptor set of the provided layout from the specified descriptor pool
     * 
     * @param descriptorSetLayout The descriptor set layout used to create the descriptor set
     * @param descriptorPool The pool from which the descriptor set is allocated
     */
    VkDescriptorSet allocateDescriptorSet(class AppDescriptorSetLayout* descriptorSetLayout);

    void destroy();
};
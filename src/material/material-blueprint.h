#pragma once
#include "material.h"

class MaterialBlueprint {
    MaterialInput* materialInput;

    // The pipeline associated with this material blueprint
    class AppPipeline* pipeline;
    class AppDescriptorSetLayout* descriptorSetLayout;
    class AppDescriptorPool* descriptorPool;

    public:
    /**
     * @brief Initializes the material blueprint
     * 
     * @note Creates a pipeline derivative that uses the specified fragment shader module
     */
    void init(class AppBase* app, class AppPipeline* pipeline);
    Material createMaterial(MaterialInput* materialInput);

};

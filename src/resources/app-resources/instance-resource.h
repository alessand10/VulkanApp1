#pragma once
#include "app-resource.h"

class AppInstance : public AppResource<VkInstance> {
    public:
    void init(class AppBase* appBase, const char* appName, bool enableValidationLayers);
    
    void destroy();
};
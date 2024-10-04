#pragma once
#include <list>
#include "vulkan-app.h"
#include "resource-list.h"

/**
 * @class AppResource
 * 
 * @brief Encapsultes a Vulkan resource, serving as a node within the linked-lists provided by the resource manager.
 * 
 * 
 */
template <typename T>
class AppResource {
    protected:
    typename std::list<T>::iterator resource;
    typename ResourceList<T>* resourceList;
    VulkanApp* app;
    bool isInitialized = false;


    public:
    void init(VulkanApp* app, typename std::list<T>::iterator resourceIt)
    {
        this->app = app;
        this->resourceList = resourceList;
        resource = resourceIt;
        isInitialized = true;
    }

    T get()
    { 
        return isInitialized ? *resource : nullptr;
    }

    T* getRef()
    {
        return &(*resource);
    }

    typename std::list<T>::iterator getIterator()
    {
        return resource; 
    }

    void setRef(typename std::list<T>::iterator it)
    {
        resource = it; 
    }

    VulkanApp* getApp()
    {
        return app;
    }
};

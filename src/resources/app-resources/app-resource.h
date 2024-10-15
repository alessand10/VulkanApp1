#pragma once
#include "resource-list.h"
#include <stdint.h>

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
    class AppBase* appBase;
    bool isInitialized = false;

    public:
    void init(class AppBase* appBase, typename std::list<T>::iterator resourceIt)
    {
        this->appBase = appBase;
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

    class AppBase* getAppBase()
    {
        return appBase;
    }
    
};


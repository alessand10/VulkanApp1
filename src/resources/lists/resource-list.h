#pragma once
#include <list>
#include "vulkan/vulkan.hpp"

template <typename T>
class ResourceList {
protected:
    typename std::list<T> resourceList;
    virtual void destroy(typename std::list<T>::iterator it) {
        resourceList.erase(it);
    }

public:
    std::list<T>::iterator create(T resource) = {
        resourceList.push_front(resource);
        return resourceList.front();
    };
};
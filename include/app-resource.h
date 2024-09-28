#pragma once
#include <list>


/**
 * @class AppResource
 * 
 * @brief Encapsultes a Vulkan resource, serving as a node within the linked-lists provided by the resource manager.
 * 
 * 
 */
template <typename T>
class AppResource {
    bool isInitialized = false;
    typename std::list<T>::iterator resource;

    public:
    T get() { return isInitialized ? *resource : nullptr; }
    typename std::list<T>::iterator getRef() {return resource; }
    void setRef(typename std::list<T>::iterator it) { resource = it; isInitialized = true; }

};
#pragma once
#include <list>


template <typename T>
class AppResource {
    bool isInitialized = false;
    typename std::list<T>::iterator resource;

    public:
    T get() { return isInitialized ? *resource : nullptr; }
    typename std::list<T>::iterator getRef() {return resource; }
    void setRef(typename std::list<T>::iterator it) { resource = it; isInitialized = true; }

};
#pragma once
#include <vector>
#include <cstdint>
#include "geometry-base.h"
#include <list>

/**
 * 
 */ 
class Mesh : public GeometryBase {
    class Material* material;
    
    public:
    void setMaterial(class Material* material);
};
#pragma once
#include "inttypes.h"

struct ViewportSettings {
    uint32_t width;
    uint32_t height;
    float minDepth;
    float maxDepth;
    float nearPlane;
    float farPlane;
    float x;
    float y;
};
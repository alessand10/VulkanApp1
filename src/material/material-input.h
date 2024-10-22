#pragma once
#include "input-texture-image.h"
#include <vector>

struct MaterialInput {
    protected:
        std::vector<MaterialInputImage> inputTextureImages = {};
    public:
    std::vector<MaterialInputImage> getInputImages() {
        return inputTextureImages;
    }
};  
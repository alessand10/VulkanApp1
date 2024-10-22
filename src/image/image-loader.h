#pragma once
#include "image.h"

class ImageLoader {
public:
    static Image loadJPEGFromFile(const std::string& filePath, uint32_t alignment);
};
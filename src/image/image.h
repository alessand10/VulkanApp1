#pragma once
#include <string>
#include <vector>
#include "inttypes.h"

class Image {
    std::vector<char> imageData;
    uint32_t width;
    uint32_t height;

    // The number o
    uint32_t rowByteAlignment;
    uint32_t bytesPerPixel;
    

public:
    // Sets this image's data
    void setData(std::vector<char> imageData) {
        this->imageData = imageData;
    }

    // Gets this image's data
    std::vector<char> getData() {
        return this->imageData;
    }

    uint32_t getHeight() {
        return this->height;
    }

    void setHeight(uint32_t height) {
        this->height = height;
    }

    uint32_t getWidth() {
        return this->width;
    }

    void setWidth(uint32_t width) {
        this->width = width;
    }

    uint32_t getByteAlignment() {
        return this->rowByteAlignment;
    }

    void setRowByteAlignment(uint32_t rowByteAlignment);

    /**
     * @brief Gets the pitch (number of bytes per row) of the image
     */
    uint32_t getPitch() {
        return this->width * this->bytesPerPixel;
    }
};
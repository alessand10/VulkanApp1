#pragma once
#include <vector>
#include <string>

struct MaterialInputImage {
    std::string name;
    class Image* image;
    MaterialInputImage(const char* name) {
        this->name = name;
    }
};

struct MaterialInputImageArray {
    std::vector<MaterialInputImage> inputImages;

    MaterialInputImageArray(std::vector<MaterialInputImage> inputImages) {
        this->inputImages = inputImages;
    }
};
#include "file-loader.h"
#include <fstream>

std::vector<char> FileLoader::loadFile(const std::string filename)
{
    std::vector<char> fileData;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    bool open = file.is_open();
    if (open) {
        size_t fileSize = (size_t)file.tellg();
        fileData.resize(fileSize);
        file.seekg(0);
        file.read(fileData.data(), fileSize);
        return fileData;
    }
}
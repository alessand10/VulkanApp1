#pragma once
#include <vector>
#include <string>

class FileLoader {
    public:
    static std::vector<char> loadFile(const std::string filename);
};
#pragma once
#include <vector>
#include <string>
#include <inttypes.h>

std::vector<char> readFile(const std::string filename);

/**
 * @brief Reads in and decompresses a JPEG image file
 * 
 * @param filename The JPEG image file
 * @param alignment The number of bytes to align to, set to -1 to have the application compute this value
 * @return std::vector<char> 
 */
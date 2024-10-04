#include "file-utilities.h"
#include <fstream>
#include "turbojpeg.h"

std::vector<char> readFile(const std::string filename)
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

std::vector<char> readJPEG(const std::string filename, int alignment, uint32_t* width, uint32_t* height)
{
    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    
    std::vector<char> jpegFile = readFile(filename);
    if (jpegFile.size() == 0) throw std::runtime_error("Failed to open jpeg image");
    const unsigned char* jpegData = static_cast<const unsigned char*>(static_cast<void*>(jpegFile.data()));
    
    int pixelFormat = TJPF_RGBA;

    // Retrieve image parameters
    tj3DecompressHeader(turboJpegHandle, jpegData, jpegFile.size());

    // Returns the image height and width in pixels, respectively
    int imageHeight = tj3Get(turboJpegHandle, TJPARAM_JPEGHEIGHT);
    int imageWidth = tj3Get(turboJpegHandle, TJPARAM_JPEGWIDTH);
    *width = static_cast<uint32_t>(imageWidth);
    *height = static_cast<uint32_t>(imageHeight);


    // Returns the number of bits used per sample
    int precision = tj3Get(turboJpegHandle, TJPARAM_PRECISION);

    int samplesPerPixel = tjPixelSize[pixelFormat];

    // Determine the number of padded pixels needed per row to satisfy memory alignment requirements
    int paddingPixelsPerRow = alignment == 0 ? 0 : (imageWidth % alignment);

    // Determine the number of samples per row, aka pitch
    int pitch = (imageWidth + paddingPixelsPerRow) * samplesPerPixel;

    // Create a vector big enough for the image
    int outputBufferSize = imageHeight * pitch;
    std::vector<char> outputImage(outputBufferSize);

    tj3Decompress8(turboJpegHandle, jpegData, jpegFile.size(), static_cast<unsigned char*>(static_cast<void*>(outputImage.data())), pitch, pixelFormat);

    return outputImage;
}
#include "image-loader.h"
#include "turbojpeg.h"
#include "file-loader.h"
#include <stdexcept>
#include "inttypes.h"

Image ImageLoader::loadJPEGFromFile(const std::string &filePath, uint32_t alignment)
{
    Image image;
    image.setRowByteAlignment(alignment);

    tjhandle turboJpegHandle = tj3Init(TJINIT_DECOMPRESS);
    
    std::vector<char> jpegFile = FileLoader::loadFile(filePath);
    if (jpegFile.size() == 0) throw std::runtime_error("Failed to open jpeg image");
    const unsigned char* jpegData = static_cast<const unsigned char*>(static_cast<void*>(jpegFile.data()));
    
    int pixelFormat = TJPF_RGBA;

    // Retrieve image parameters
    tj3DecompressHeader(turboJpegHandle, jpegData, jpegFile.size());

    // Returns the image height and width in pixels, respectively
    int imageHeight = tj3Get(turboJpegHandle, TJPARAM_JPEGHEIGHT);
    int imageWidth = tj3Get(turboJpegHandle, TJPARAM_JPEGWIDTH);

    image.setWidth(imageWidth);
    image.setHeight(imageHeight);

    // Returns the number of bits used per sample
    int precision = tj3Get(turboJpegHandle, TJPARAM_PRECISION);

    int samplesPerPixel = tjPixelSize[pixelFormat];

    // Determine the number of padded pixels needed per row to satisfy memory alignment requirements
    int paddingPixelsPerRow = alignment == 0 ? 0 : (imageWidth % alignment);

    // Determine the number of samples per row, aka pitch
    int pitch = (imageWidth + paddingPixelsPerRow) * samplesPerPixel;

    // Create a vector big enough for the image
    int outputBufferSize = imageHeight * pitch;
    std::vector<char> outputImageData(outputBufferSize);

    tj3Decompress8(turboJpegHandle, jpegData, jpegFile.size(), static_cast<unsigned char*>(static_cast<void*>(outputImageData.data())), pitch, pixelFormat);

    image.setData(outputImageData);

    return image;
}
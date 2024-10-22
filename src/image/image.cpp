#include "image.h"

void Image::setRowByteAlignment(uint32_t rowByteAlignment)
{
    uint32_t oldByteAlignment = this->rowByteAlignment;

    // Return immediately if the alignment has not changed
    if (rowByteAlignment == oldByteAlignment) return; 

    // Calculate the old number of bytes used for padding
    uint32_t oldPaddingBytes = oldByteAlignment == 0U ? 0U : (width * bytesPerPixel) % oldByteAlignment;

    // Calculate the new number of bytes used for padding
    uint32_t newPaddingBytes = rowByteAlignment == 0U ? 0U : (width * bytesPerPixel) % rowByteAlignment;

    uint32_t newPaddingPixels = newPaddingBytes / bytesPerPixel;

    uint32_t newRowPitch = (width) * bytesPerPixel;

    std::vector<char> newImageData(newRowPitch * height);

    for (uint32_t row = 0U ; row < height ; row++) {
        uint32_t rowPitch = width * bytesPerPixel;
        uint32_t rowByteOffset = row * rowPitch;
    }
}
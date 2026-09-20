#pragma once
#include "pixel.h"
#include <vector>

/**
 * Class that represents an image in memory.
 *
 * Provides access to individual pixels, see the Pixel class.
 *
 * Use the "load_image" function in the file "load.h" to load images from disk.
 */
class Image {
public:
    // Create an empty image. Initially, all pixels are black.
    Image(size_t width, size_t height);

    // Get width and height.
    size_t width() const { return m_width; }
    size_t height() const { return m_height; }

    // Get a pixel at a specific coordinate.
    Pixel &pixel(size_t x, size_t y) {
        return pixels[x + y*width()];
    }

    // Const version of the function above.
    const Pixel &pixel(size_t x, size_t y) const {
        return pixels[x + y*width()];
    }

    // Shrink the image to the specified dimensions.
    // Note: width and height must be smaller than or equal to the original dimensions.
    Image shrink(size_t width, size_t height) const;

    // Compute similarity to another image.
    double compare_to(const Image &other) const;

    // Compute the average intensity of this image.
    double average_brightness() const;

private:
    // Size of the image.
    size_t m_width;
    size_t m_height;

    // Pixel data.
    std::vector<Pixel> pixels;
};

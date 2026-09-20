#pragma once
#include <iostream>
#include <algorithm>

/**
 * A single pixel in an image.
 *
 * Expressed as three RGB numbers, each between 0 and 1.
 */
class Pixel {
public:
    // Create a black pixel.
    Pixel() : r(0), g(0), b(0) {}

    // Create from floating point values.
    Pixel(float r, float g, float b)
        : r(r), g(g), b(b) {}

    // Create from three byte values in the range 0-255.
    static Pixel from_bytes(int r, int g, int b) {
        return Pixel(r / 255.0f, g / 255.0f, b / 255.0f);
    }

    // Pixel values for each of red, green, and blue.
    float r;
    float g;
    float b;

    // Compute the brightness of the pixel.
    float brightness() const {
        return (r + g + b) / 3;
    }

    // Add two pixel values to each other.
    Pixel operator +(const Pixel &o) const {
        return Pixel(r + o.r, g + o.g, b + o.b);
    }

    // Multiply a pixel with a constant.
    Pixel operator *(float c) const {
        return Pixel(r * c, g * c, b * c);
    }

    // Divide a pixel value with a constant.
    Pixel operator /(float c) const {
        return Pixel(r / c, g / c, b / c);
    }
};

// Output a pixel.
std::ostream &operator <<(std::ostream &to, const Pixel &px);

// Alternate version of multiplication with a constant.
inline Pixel operator *(float c, const Pixel &o) {
    return o * c;
}

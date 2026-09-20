#include "image.h"
#include <iostream>
#include <cassert>
#include <cmath>

Image::Image(size_t width, size_t height)
    : m_width(width),
      m_height(height),
      pixels(width*height, Pixel()) {}

// Halve the size of an image. Helper function to "shrink" below.
static Image half_size(const Image &src) {
    Image out(src.width() / 2, src.height() / 2);

    for (size_t y = 0; y < src.height() - 1; y += 2) {
        for (size_t x = 0; x < src.width() - 1; x += 2) {
            Pixel result =
                src.pixel(x, y) +
                src.pixel(x + 1, y) +
                src.pixel(x, y + 1) +
                src.pixel(x + 1, y + 1);
            out.pixel(x / 2, y / 2) = result / 4;
        }
    }

    return out;
}

// Halve the width of an image. Helper function to "shrink" below.
static Image half_width(const Image &src) {
    Image out(src.width() / 2, src.height());

    for (size_t y = 0; y < src.height(); y++) {
        for (size_t x = 0; x < src.width() - 1; x += 2) {
            Pixel result =
                src.pixel(x, y) +
                src.pixel(x + 1, y);
            out.pixel(x / 2, y) = result / 2;
        }
    }

    return out;
}

// Halve the height of an image. Helper function to "shrink" below.
static Image half_height(const Image &src) {
    Image out(src.width(), src.height() / 2);

    for (size_t y = 0; y < src.height() - 1; y += 2) {
        for (size_t x = 0; x < src.width(); x++) {
            Pixel result =
                src.pixel(x, y) +
                src.pixel(x, y + 1);
            out.pixel(x, y / 2) = result / 2;
        }
    }

    return out;
}

// Linear interpolation between two pixels values.
static Pixel lerp(Pixel a, Pixel b, float t) {
    return (1 - t)*a + t*b;
}

// Shrink the size of an image using billinear interpolation. Assumes that the size of "src" is less
// than or equal to double the size indicated in "width" and "height". If this is not true,
// interpolation will still work, but the resulting image will not look good.
static Image billinear(const Image &src, size_t width, size_t height) {
    Image out(width, height);

    for (size_t y = 0; y < out.height(); y++) {
        for (size_t x = 0; x < out.width(); x++) {
            float orig_x = float(x) * src.width() / width;
            float orig_y = float(y) * src.height() / height;

            float whole_x, whole_y;
            float frac_x = std::modf(orig_x, &whole_x);
            float frac_y = std::modf(orig_y, &whole_y);

            size_t int_x = static_cast<size_t>(whole_x);
            size_t int_y = static_cast<size_t>(whole_y);

            Pixel result;
            if (int_x + 1 >= src.width()) {
                if (int_y + 1 >= src.height()) {
                    result = src.pixel(int_x, int_y);
                } else {
                    result = lerp(src.pixel(int_x, int_y), src.pixel(int_x, int_y + 1), frac_y);
                }
            } else {
                if (int_y + 1 >= src.height()) {
                    result = lerp(src.pixel(int_x, int_y), src.pixel(int_x + 1, int_y), frac_x);
                } else {
                    Pixel a = lerp(src.pixel(int_x, int_y), src.pixel(int_x, int_y + 1), frac_y);
                    Pixel b = lerp(src.pixel(int_x + 1, int_y), src.pixel(int_x + 1, int_y + 1), frac_y);
                    result = lerp(a, b, frac_x);
                }
            }

            out.pixel(x, y) = result;
        }
    }

    return out;
}

Image Image::shrink(size_t width, size_t height) const {
    assert(width <= this->width());
    assert(height <= this->height());

    Image result = *this;

    // Halve both dimensions:
    while (result.width() / 2 >= width && result.height() / 2 >= height)
        result = half_size(result);

    // Halve width:
    while (result.width() / 2 >= width)
        result = half_width(result);

    // Halve height:
    while (result.height() / 2 >= height)
        result = half_height(result);

    // Finally, if size is not exact, then just use linear interpolation of the remaining pixels.
    if (result.height() != height || result.width() != width)
        result = billinear(result, width, height);

    return result;
}

// Compute a sum of a (large) vector of doubles, while keeping the rounding error fairly small.
static double reduce_sum(std::vector<double> data) {
    while (data.size() > 1) {
        std::vector<double> reduced;
        reduced.reserve(data.size() / 2);

        for (size_t i = 0; i < data.size() - 1; i += 2)
            reduced.push_back(data[i] + data[i + 1]);

        // Odd number of elements?
        if (data.size() % 2 != 0)
            reduced.back() += data.back();

        std::swap(data, reduced);
    }

    return data[0];
}

double Image::average_brightness() const {
    // This is a bit tricky: we need to keep the floating point error relatively small.
    std::vector<double> brightness;
    brightness.reserve(pixels.size());
    for (size_t i = 0; i < pixels.size(); i++)
        brightness.push_back(pixels[i].brightness());

    return reduce_sum(brightness) / float(pixels.size());
}

// Helper function to "compare_to" below. Assumes dimensions of the two images are the same.
static double compare(const Image &a, const Image &b) {
    size_t width = a.width();
    size_t height = a.height();

    double brightness_a = a.average_brightness();
    double brightness_b = b.average_brightness();

    std::vector<double> delta;
    delta.reserve(width * height);

    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            double norm_a = a.pixel(x, y).brightness() / brightness_a;
            double norm_b = b.pixel(x, y).brightness() / brightness_b;
            delta.push_back(pow(norm_a - norm_b, 2));
        }
    }

    return reduce_sum(delta) / float(width * height);
}

double Image::compare_to(const Image &other) const {
    size_t min_width = std::min(width(), other.width());
    size_t min_height = std::min(height(), other.height());

    if ((min_width != width() || min_height != height()) &&
        (min_width != other.width() || min_height != other.height())) {
        return compare(shrink(min_width, min_height), other.shrink(min_width, min_height));
    } else if (min_width != width() || min_height != height()) {
        return compare(shrink(min_width, min_height), other);
    } else if (min_width != other.width() || min_height != other.height()) {
        return compare(*this, other.shrink(min_width, min_height));
    } else {
        return compare(*this, other);
    }
}

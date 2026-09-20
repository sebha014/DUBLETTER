#pragma once
#include "image.h"
#include <string>
#include <memory>

/**
 * Window for showing images.
 */
class Window {
public:
    // Create.
    Window();

    // Virtual destructor.
    virtual ~Window() = default;

    // Disable pausing globally.
    bool disable_pause;

    // Create.
    static std::unique_ptr<Window> create(int &argc, const char **argv);

    // Show a single image.
    void show_single(const Image &image, bool pause = true);
    void show_single(const std::string &caption, const Image &image, bool pause = true);

    // Show two images side-by-side.
    void show_pair(const Image &image1, const Image &image2, bool pause = true);
    void show_pair(const std::string &caption1, const Image &image1,
                const std::string &caption2, const Image &image2, bool pause = true);

    // Show multiple images side-by-side.
    void show_multiple(const std::vector<Image> &images, bool pause = true);
    virtual void show_multiple(const std::vector<std::string> &captions,
                            const std::vector<Image> &images,
                            bool pause = true) = 0;

    // Report a match between two images.
    void report_match(const std::vector<std::string> &files);
};

using WindowPtr = std::unique_ptr<Window>;

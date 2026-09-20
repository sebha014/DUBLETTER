#include "window.h"
#include "os.h"
#include "load.h"
#include "sfml_window.h"
#include "win32_window.h"
#include <iostream>
#include <cstring>

Window::Window() : disable_pause(false) {}

void Window::show_single(const Image &image, bool pause) {
    show_single("", image, pause);
}

void Window::show_single(const std::string &caption, const Image &image, bool pause) {
    show_multiple(std::vector<std::string>(1, caption), std::vector<Image>(1, image), pause);
}

void Window::show_pair(const Image &image1, const Image &image2, bool pause) {
    show_pair("", image1, "", image2, pause);
}

void Window::show_pair(const std::string &caption1, const Image &image1,
                    const std::string &caption2, const Image &image2, bool pause) {
    std::vector<std::string> captions;
    captions.push_back(caption1);
    captions.push_back(caption2);
    std::vector<Image> images;
    images.push_back(image1);
    images.push_back(image2);
    show_multiple(captions, images, pause);
}

void Window::show_multiple(const std::vector<Image> &images, bool pause) {
    show_multiple(std::vector<std::string>(images.size(), ""), images, pause);
}

void Window::report_match(const std::vector<std::string> &files) {
    std::cout << "MATCH: ";
    for (size_t i = 0; i < files.size(); i++) {
        if (i > 0)
            std::cout << ", ";
        std::cout << files[i];
    }
    std::cout << std::endl;

    std::vector<Image> images;
    images.reserve(files.size());
    for (size_t i = 0; i < files.size(); i++)
        images.push_back(load_image(files[i]));

    show_multiple(files, images, true);
}

class No_Window : public Window {
public:
    No_Window() {}

    virtual void show_multiple(const std::vector<std::string> &,
                            const std::vector<Image> &,
                            bool) {}

    virtual void wait() {}
};

static void remove_arg(int &argc, const char **argv) {
    for (int i = 2; i < argc; i++) {
        argv[i - 1] = argv[i];
    }
    argc--;
    argv[argc] = nullptr;
}

std::unique_ptr<Window> Window::create(int &argc, const char **argv) {
    std::unique_ptr<Window> result;
    bool no_pause = false;

    while (argc > 2) {
        if (strcmp(argv[1], "--nopause") == 0) {
            no_pause = true;
            remove_arg(argc, argv);
        } else if (strcmp(argv[1], "--nowindow") == 0) {
            remove_arg(argc, argv);
            result = std::make_unique<No_Window>();
        } else {
            break;
        }
    }

    if (!result) {
#ifdef WINDOWS
        result = std::make_unique<Win32_Window>();
#else
        result = std::make_unique<SFML_Window>();
#endif
    }

    if (no_pause)
        result->disable_pause = true;
    return result;
}

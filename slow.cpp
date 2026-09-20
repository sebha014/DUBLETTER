#include "image.h"
#include "window.h"
#include "load.h"
#include <chrono>
#include <string>
#include <vector>
#include <iostream>

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;

/**
 * Parameters for the comparison:
 */

// How small shall we make the images before comparing them?
const size_t image_size = 32;

// What is the threshold we should use to consider images equal enough?
const double threshold = 0.01;


int main(int argc, const char *argv[]) {
    WindowPtr window = Window::create(argc, argv);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " [--nopause] [--nowindow] <directory>" << endl;
        cerr << "Missing directory containing files!" << endl;
        return 1;
    }

    vector<string> files = list_files(argv[1]);
    cout << "Found " << files.size() << " image files." << endl;

    if (files.size() <= 0) {
        cerr << "No files found! Make sure you entered a proper path!" << endl;
        return 1;
    }

    auto begin = std::chrono::high_resolution_clock::now();

    /**
     * Step 1: Load and shrink all images:
     *
     * Note: We shrink images already here so that all images fit in RAM.
     * Re-loading in step 2 would take too much time.
     */

    window->show_single("Loading images...", load_image(files[0]), false);

    vector<Image> images;
    for (const auto &file : files)
        images.push_back(load_image(file).shrink(image_size, image_size));

    auto load_time = std::chrono::high_resolution_clock::now();
    cout << "Loading images took: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(load_time - begin).count()
         << " milliseconds." << endl;

    /**
     * Step 2: Find duplicates:
     */

    window->show_single("Comparing images...", load_image(files[1]), false);

    for (size_t i = 0; i < images.size(); i++) {
        // Already shown?
        if (files[i].empty())
            continue;

        // Collect all pictures that match this one.
        vector<string> matches;
        matches.push_back(files[i]);

        cout << "Examining " << files[i] << "..." << endl;

        for (size_t j = i + 1; j < images.size(); j++) {
            double difference = images[i].compare_to(images[j]);
            if (difference <= threshold) {
                matches.push_back(files[j]);
                // Set it to the empty string so that we don't show the same
                // picture more than once.
                files[j] = "";
            }
        }

        if (matches.size() > 1)
            window->report_match(matches);
    }

    auto end = std::chrono::high_resolution_clock::now();
    cout << "Total time: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
         << " milliseconds." << endl;

    return 0;
}

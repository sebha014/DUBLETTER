#include "image.h"
#include "window.h"
#include "load.h"
#include <chrono>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include "hash_map.h"

using std::cout;
using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::unordered_map;

/**
 * Class that stores a summary of an image.
 *
 * This summary is intended to contain a high-level representation of the
 * important parts of an image. I.e. it shall contain what a human eye would
 * find relevant, while ignoring things that the human eye would find
 * irrelevant.
 *
 * To approximate human perception, we store a series of booleans that indicate
 * if the brightness of the image has increased or not. We do this for all
 * horizontal lines and vertical lines in a downsampled version of the image.
 *
 * See the lab instructions for more details.
 *
 * Note: You will need to use this data structure as the key in a hash table. As
 * such, you will need to implement equality checks and a hash function for this
 * data structure.
 */
/////////////////////////////////
class Image_Summary {
public:
    // Horizontal increases in brightness.
    vector<bool> horizontal;

    // Vertical increases in brightness.
    vector<bool> vertical;

    bool operator==(const Image_Summary &other) const {
        return horizontal == other.horizontal &&
               vertical == other.vertical;
    }
};
///////////////////////////////////////////////
template <>
class std::hash<Image_Summary> {
public:
    size_t operator()(const Image_Summary &to_hash) const {
        size_t result = 0;

        for (bool value : to_hash.horizontal) {
            result = (result << 1) | value;
        }

        for (bool value : to_hash.vertical) {
            result = (result << 1) | value;
        }

        return result;
    }
};

////////////////////////////////////////////////
// Compute an Image_Summary from an image. This is described in detail in the
// lab instructions.
Image_Summary compute_summary(const Image &image) {
    const size_t summary_size = 8;
    Image_Summary result;

    Image small = image.shrink(summary_size + 1, summary_size + 1);

    for (size_t y = 0; y < summary_size + 1; y++) {
    for (size_t x = 0; x < summary_size; x++) {
        result.horizontal.push_back(
            small.pixel(x + 1, y).brightness() >
            small.pixel(x, y).brightness()
        );
    }
}

    for (size_t x = 0; x < summary_size + 1; x++) {
    for (size_t y = 0; y < summary_size; y++) {
        result.vertical.push_back(
            small.pixel(x, y + 1).brightness() >
            small.pixel(x, y).brightness()
        );
    }
}

    return result;
}
////////////////////////////////////////////////////////

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
     * TODO:
     * - For each file:
     *   - Load the file
     *   - Compute its summary
     */

    Hash_Map<Image_Summary, vector<string>> map;

    for (const string &file : files) {
        Image image = load_image(file);
        Image_Summary summary = compute_summary(image);

        map[summary].push_back(file);
    }


    auto end = std::chrono::high_resolution_clock::now();
    cout << "Total time: "
         << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count()
         << " milliseconds." << endl;

    /**
     * TODO:
     * - Display sets of files with equal summaries
     */

    for (auto it = map.begin(); it != map.end(); ++it) {
        auto [summary, filenames] = *it;

        if (filenames.size() >= 2) {
            window->report_match(filenames);
        }      
    }
    
    return 0;
}

#include "hash_map.h"
#include <unordered_map>
#include "hash_info.h"

#include <iostream>
#include <iomanip>
#include <random>
#include <chrono>
#include <string>
#include <vector>
#include <limits>
#include <unordered_set>

/**
 * Main test program for hash tables.
 *
 * Can be used as a basic test harness for the Hash_Map implementation. If you
 * find a bug in your implementation, it might be faster to test by writing your
 * own small test program that just adds and/or removes values from your hash
 * table.
 */


/**
 * Key/value types to use for testing. The rest of the program below assumes
 * that it is possible to read these types from stdin using cin >> x, and print
 * them using cout << x. Remember that the Hash_Map assumes that the types have
 * a default constructor as well.
 *
 * For example, try to replace one (or both) with std::string.
 */
using Key_Type = int;
using Value_Type = int;

/**
 * Some using statements to make things easier to work with below.
 */
using std::cout;
using std::cin;
using std::setw;
using std::endl;
using std::make_pair;
using std::unordered_map;
using std::string;
using std::vector;

/**
 * Results from a benchmark.
 */
struct Benchmark_Results {
    // Times for insert, lookup, and removal.
    long insert, lookup, remove;

    // Statistics about the hash table.
    size_t max_buckets, max_chain;
    float load_factor;
};

// Run the benchmark for one of the hash maps.
template <typename Map>
Benchmark_Results run_benchmark(const vector<int> &elements) {
    Benchmark_Results results{};

    auto start = std::chrono::high_resolution_clock::now();

    Map map;

    // Insert elements:
    for (size_t i = 0; i < elements.size(); i++) {
        map.insert(make_pair(elements[i], static_cast<int>(i)));
    }

    auto after_insert = std::chrono::high_resolution_clock::now();

    // Look up all elements:
    for (size_t i = 0; i < elements.size(); i++) {
        int found = map.at(elements[i]);
        if (found != static_cast<int>(i)) {
            cout << "ERROR: Expected " << i << " but got " << found << endl;
        }
    }

    auto after_lookup = std::chrono::high_resolution_clock::now();

    results.max_buckets = map.bucket_count();
    results.max_chain = max_chain_length(map);
    results.load_factor = map.load_factor();

    // For debugging smaller cases:
    // debug_print(map);

    // Remove all elements:
    for (size_t i = 0; i < elements.size(); i++) {
        if (!map.erase(elements[i])) {
            cout << "ERROR: Removal of " << elements[i] << " failed." << endl;
        }
    }

    auto after_remove = std::chrono::high_resolution_clock::now();

    results.insert = std::chrono::duration_cast<std::chrono::milliseconds>(after_insert - start).count();
    results.lookup = std::chrono::duration_cast<std::chrono::milliseconds>(after_lookup - after_insert).count();
    results.remove = std::chrono::duration_cast<std::chrono::milliseconds>(after_remove - after_lookup).count();
    return results;
}

// Run a small benchmark comparing Hash_Map and std::unordered_map.
void benchmark() {
    // Set the seed value to something. That way we get the same result every
    // time. Feel free to change the seed if you want.
    std::mt19937 random(500);

    // Number of elements to generate.
    const size_t num_elements = 1000000;

    // Generate the elements ahead of time. Avoid duplicates.
    std::unordered_set<int> used;
    vector<int> elements;
    std::uniform_int_distribution distribution(0, std::numeric_limits<int>::max());
    for (size_t i = 0; i < num_elements; i++) {
        int value = distribution(random);

        // Found a duplicate, try again.
        while (used.count(value) > 0)
            value = distribution(random);

        // Add to our elements, and remember that we used the value.
        elements.push_back(value);
        used.insert(value);
    }

    // Run the benchmark!
    cout << "Running benchmark for std::unordered_map..." << endl;
    Benchmark_Results unordered = run_benchmark<std::unordered_map<int, int>>(elements);
    cout << "Running benchmark for Hash_Map..." << endl;
    Benchmark_Results custom = run_benchmark<Hash_Map<int, int>>(elements);

    cout << "Results:" << endl;
    int width0 = 11;
    int width1 = 14;
    int width2 = 9;
    cout << setw(width0) << "|" << " unordered_map | Hash_Map" << endl;
    cout << setw(width0) << "insert |"
         << setw(width1 - 3) << unordered.insert << " ms |"
         << setw(width2 - 3) << custom.insert << " ms" << endl;
    cout << setw(width0) << "lookup |"
         << setw(width1 - 3) << unordered.lookup << " ms |"
         << setw(width2 - 3) << custom.lookup << " ms" << endl;
    cout << setw(width0) << "remove |"
         << setw(width1 - 3) << unordered.remove << " ms |"
         << setw(width2 - 3) << custom.remove << " ms" << endl;
    cout << setw(width0) << "buckets |"
         << setw(width1) << unordered.max_buckets << " |"
         << setw(width2) << custom.max_buckets << endl;
    cout << setw(width0) << "max chain |"
         << setw(width1) << unordered.max_chain << " |"
         << setw(width2) << custom.max_chain << endl;
    cout << setw(width0) << "load |"
         << setw(width1) << unordered.load_factor << " |"
         << setw(width2) << custom.load_factor << endl;
}

// Main loop of the program. Accepts the map to use as a template so that we can
// test both std::unordered_map as well as our Hash_Map!
template <typename Map>
void menu() {
    Map map;

    // This is a good place to insert some data if you wish to speed up testing!
    // map.insert(make_pair(2, 8));

    cout << "1 - Insert an element.\n";
    cout << "2 - Find an element.\n";
    cout << "3 - Remove an element.\n";
    cout << "4 - Clear table and set size.\n";
    cout << "5 - Iterate and print the hash table.\n";
    cout << "6 - Print the structure of the hash table.\n";
    cout << "9 - Run the benchmark.\n";
    cout << "0 - Exit.\n";

    Key_Type key;
    Value_Type value;
    size_t size;

    int choice;
    while (cout << "\nChoice: ", cin >> choice) {
        try {
            switch (choice) {
            case 0:
                return;
            case 1:
                cout << "Key to insert: ";
                cin >> key;
                cout << "Value to insert: ";
                cin >> value;

                map.insert(make_pair(key, value));
                break;
            case 2:
                cout << "Key to find: ";
                cin >> key;

                cout << "map.find(" << key << ") = ";
                if (auto elem = map.find(key); elem != map.end()) {
                    cout << "( " << (*elem).first << " -> " << (*elem).second << " )" << endl;
                } else {
                    cout << "<no element>" << endl;
                }
                break;
            case 3:
                cout << "Key to remove: ";
                cin >> key;

                if (map.erase(key) == 0) {
                    cout << "Key not found." << endl;
                } else {
                    cout << "Key removed." << endl;
                }
                break;
            case 4:
                cout << "Enter new hash table size: ";
                if (cin >> size) {
                    map = Map(size);
                    cout << "OK" << endl;
                } else {
                    cout << "Invalid input." << endl;
                }
                break;
            case 5:
                cout << "The map contains " << map.size() << " elements:";
                for (auto &&k : map) {
                    cout << " ( " << k.first << " -> " << k.second << " )";
                }
                cout << endl;
                break;
            case 6:
                cout << "Contents of the map:" << endl;
                debug_print(map);
                break;
            case 9:
                cout << "Running benchmark..." << endl;
                benchmark();
                cout << "Done!" << endl;
                break;
            default:
                cout << "Unknown option: " << choice << endl;
                break;
            }
        } catch (const std::exception &e) {
            cout << e.what() << endl;
        } catch (...) {
            cout << "Caught an unknown exception!" << endl;
        }
    }
}


/**
 * Main part of the program.
 */
int main(int argc, const char *argv[]) {
    if (argc > 1) {
        if (string("std") == argv[1]) {
            // Run the menu with std::unordered_map.
            menu<unordered_map<Key_Type, Value_Type>>();
        } else if (string("custom") == argv[1]) {
            // Run the menu with our custom map.
            menu<Hash_Map<Key_Type, Value_Type>>();
        } else {
            cout << "Unknown argument to the program!" << endl;
            return 1;
        }
    } else {
        // Run the menu with our custom map.
        menu<Hash_Map<Key_Type, Value_Type>>();
    }

    return 0;
}

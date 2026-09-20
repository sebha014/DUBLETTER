#pragma once

/**
 * This file implements the utility function 'debug_print', both for Hash_Map
 * and for std::unordered_map.
 *
 * As long as you don't change the data representation in Hash_Map, you don't
 * need to change anything here.
 */

#include "hash_map.h"
#include <unordered_map>

#include <iostream> // for cout, endl
#include <iomanip>  // for setw
#include <utility>  // for min, max

/**
 * Get maximum chain length.
 */

// Get the maximum chain length for Hash_Map.
template <typename Key, typename Value>
size_t max_chain_length(const Hash_Map<Key, Value> &m) {
    size_t max_chain = 0;

    size_t start = 0;
    while (start < m.bucket_count()) {
        // Count the chain.
        size_t curr_chain = 0;
        size_t end = start;
        while (m.bucket(end)) {
            end = (end + 1) % m.bucket_count();
            curr_chain++;
        }

        max_chain = std::max(max_chain, curr_chain);
        // Note: We *don't* account for wraparound here - to make sure to exit.
        start = start + curr_chain + 1;
    }

    return max_chain;
}

// Get the maximum chain length for std::unordered_map.
template <typename Key, typename Value>
size_t max_chain_length(const std::unordered_map<Key, Value> &m) {
    size_t max_chain = 0;
    for (size_t i = 0; i < m.bucket_count(); i++) {
        size_t curr_chain = 0;
        for (auto bucket = m.begin(i); bucket != m.end(i); ++bucket) {
            curr_chain++;
        }
        max_chain = std::max(max_chain, curr_chain);
    }
    return max_chain;
}


/**
 * Print information.
 */

// Print the contents of a Hash_Map.
template <typename Key, typename Value>
void debug_print(const Hash_Map<Key, Value> &m) {
    std::cout << "capacity = " << m.bucket_count()
              << ", elements = " << m.size()
              << ", current load = " << m.load_factor()
              << ", max chain = " << max_chain_length(m)
              << std::endl;
    for (size_t i = 0; i < m.bucket_count(); i++) {
        std::cout << std::setw(4) << i << ": ";
        if (auto value = m.bucket(i)) {
            std::cout << " ( " << value->first << " -> " << value->second << " )";
        }
        std::cout << std::endl;
    }
}

// Print the contents of a std::unordered_map.
template <typename Key, typename Value>
void debug_print(const std::unordered_map<Key, Value> &m) {
    std::cout << "bucket count = " << m.bucket_count()
              << ", elements = " << m.size()
              << ", average load = " << m.load_factor()
              << ", max chain = " << max_chain_length(m)
              << std::endl;
    // Print the contents of the buckets.
    for (size_t i = 0; i < m.bucket_count(); i++) {
        std::cout << std::setw(4) << i << ":";
        for (auto bucket = m.begin(i); bucket != m.end(i); ++bucket) {
            std::cout << " ( " << bucket->first << " -> " << bucket->second << " )";
        }
        std::cout << std::endl;
    }
}

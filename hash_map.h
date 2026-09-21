#pragma once
#include <cstddef>    // for size_t
#include <utility>    // for std::pair, std::min, std::max
#include <cassert>    // for assert
#include <functional> // for std::hash
#include <stdexcept>  // for std::runtime_error
#include <optional>   // for std::optional

/**
 * Implementation of a simple hash map that uses open addressing and linear
 * probing.
 *
 * For simplicity, we assume that both Key and Value have default constructors.
 * That way we do not have to worry about managing the lifetime of individual
 * elements ourselves (you are of course allowed to try if you want to).
 */
template <typename Key, typename Value>
class Hash_Map {
public:
    // There is a basic iterator at the bottom.
    class const_iterator;

    // Create an empty hash map. Optionally specify the initial capacity and load factor.
    Hash_Map(size_t capacity = 10, float load = 0.8) {
        // Set a minimum capacity of 4.
        capacity = std::max(capacity, static_cast<size_t>(4));

        // Limit the load factor to sensible values.
        max_load_factor = std::max(std::min(load, 1.0f), 0.2f);

        // Create the table. Also initializes the rest of the member variables.
        alloc_table(capacity);
    }

    // Destroy the hash map.
    ~Hash_Map() {
        delete[] used;
        delete[] keys;
        delete[] values;
    }

    // Move constructor.
    Hash_Map(Hash_Map &o)
        : used{o.used}, keys{o.keys}, values{o.values},
          capacity{o.capacity}, element_count{o.element_count},
          max_load_factor{o.max_load_factor} {

        o.used = nullptr;
        o.keys = nullptr;
        o.values = nullptr;
    }

    // Move assignment.
    Hash_Map &operator =(Hash_Map &&o) {
        std::swap(used, o.used);
        std::swap(keys, o.keys);
        std::swap(values, o.values);
        std::swap(capacity, o.capacity);
        std::swap(element_count, o.element_count);
        std::swap(max_load_factor, o.max_load_factor);
        return *this;
    }

    // We don't support copy. You are of course free to implement the operators if you wish.
    Hash_Map(const Hash_Map &o) = delete;
    Hash_Map &operator =(const Hash_Map &o) = delete;

    // Add an element to the hash map. Note: we don't follow the interface of
    // std::map and std::unordered_map exactly here (they return a pair with an iterator).
    bool insert(const std::pair<const Key, Value> &elem) {
        size_t pos = insert_key(elem.first, elem.second);
        return pos < capacity;
    }

    // Check if a specific key is in the map. Like std::map and
    // std::unordered_map, this should return 0 if the key was not found and 1
    // otherwise.
    size_t count(const Key &key) {
        size_t pos = find_key(key);
        if (pos < capacity)
            return 1;
        else
            return 0;
    }

    // Get an element from the hash map.
    Value &at(const Key &key) {
        size_t pos = find_key(key);
        if (pos < capacity) {
            return values[pos];
        } else {
            throw std::out_of_range("key not found");
        }
    }

    // Const version.
    Value &at(const Key &key) const {
        size_t pos = find_key(key);
        if (pos < capacity) {
            return values[pos];
        } else {
            throw std::out_of_range("key not found");
        }
    }

    // Get an iterator to a key.
    const_iterator find(const Key &key) const {
        size_t pos = find_key(key);
        if (pos >= capacity)
            pos = capacity;
        return const_iterator(*this, pos);
    }

    // Version of 'at' that inserts a new element if the key was not found.
    Value &operator [](const Key &key) {
        size_t pos = find_key(key);
        if (pos >= capacity) {
            // Not found, we need to insert.
            pos = insert_key(key, Value());

            if (pos >= capacity) {
                // This should not happen: 'find_key' said the key did not
                // exist, so if 'insert_key' says it could not insert the key
                // because it already exists, at least one of the function is
                // lying!
                throw std::runtime_error("failed to insert a key that 'find_key' could not find");
            }
        }
        return values[pos];
    }

    // Remove an element from the map. Returns the number of elements removed
    // (i.e. 0 or 1). Note: there are a few other overloads in std::map and
    // std::unordered_map which we don't implement.
    size_t erase(const Key &key) {
        bool ok = erase_key(key);
        if (ok)
            return 1;
        else
            return 0;
    }

    // Get the number of elements.
    size_t size() const {
        return element_count;
    }

    // Get number of buckets (name is perhaps misleading, it is chosen to be
    // compatible with std::unordered_map).
    size_t bucket_count() const {
        return capacity;
    }

    // Get current load factor.
    float load_factor() const {
        return static_cast<float>(element_count) / static_cast<float>(capacity);
    }

private:
    /**
     * One way to implement a hash map is to store elements in a big array where
     * each element is a struct { bool used; Key key; Value value; }. However,
     * if we need to probe multiple elements, it is typically more efficient to
     * store the three elements of our struct as three different arrays. This
     * both makes the data representation more compact, but also means that the
     * values (which we will access less frequently than the keys) do not take
     * up valuable space in the cache.
     *
     * As such, the three arrays below (used, keys, values) should be considered
     * to belong together. For example used[1] indicates whether keys[1] and
     * values[1] contain "real" values or not (remember: since we don't know
     * anything about Key we can't see if it is "empty").
     */

    // Remember which elements are used.
    bool *used;

    // All keys.
    Key *keys;

    // All values.
    Value *values;

    // Remember the total capacity of all three arrays above.
    size_t capacity;

    // Remember how many elements are actually used. We use this information to
    // determine when we need to resize the arrays.
    size_t element_count;

    // Load factor. Between 0 and 1. Determines how full we allow the array to
    // get before we resize it.
    float max_load_factor;

    // Instance of std::hash for the key element type. See
    // https://en.cppreference.com/cpp/container/unordered_map/unordered_map for
    // an example of how to override std::hash for custom data types (done in
    // part 2 of the lab).
    std::hash<Key> hash_function;

    // Helper function to hash an element. By default we use std::hash, just
    // like std::unordered_map. Note that std::hash for integers (at least on
    // Linux) just returns the number that is passed to the hash function.
    size_t hash_key(const Key &k) const {
        // Call the hash function. This generates a value from 0 to
        // std::numeric_limits<size_t>::max() (about 2^64).
        size_t h = hash_function(k);

        // Adapt the size to the capacity of our table.
        return h % capacity;
    }

    // Insert an element. If successful, the element's position is returned. If
    // there is already another element with the same key in the table, return
    // some value >= capacity.
    /////////////////////////////////////////////////////
    ///////////////////INSERT_KEY//////////////////////////
    ////////////////////////////////////////////////////

    size_t insert_key(const Key &key, const Value &value) {
        // Grow the table if needed. Ensures that there are at least 2 free
        // elements, making sure that at least one element is empty after we
        // insert an element.
        grow_if_needed(); //KIKA TILL LITE PÅ DENNA

        //Börjar på den position som key hachas till
        size_t pos = hash_key(key);

        //fortsätt söka så länge platsen är upptagen
        while (used[pos]) {

            //Om key finns ska vi inte lägga in den igen
            if (keys[pos] == key) {
                return capacity;
            }

            //Kollision -> gå till nästa position.
            //& capacity gör att vi börjar om från 0 efter sista platsen.
            pos = (pos + 1) % capacity;
        }

        //Hittade en ledig plats så läggs key och value in
        used[pos] = true;
        keys[pos] = key;
        values[pos] = value;

        //Vi har lagt till ett nytt element ökar count
        element_count++;

        //retunerar position där element läggs in
        return pos;
    }
    /////////////////////////////////////////////////
    /////////////////////////////////////////////////

    // Find which the position where a key is located in the hash table. If the
    // key is found, return its position in the table. Otherwise, returns some
    // value >= capacity.
    /////////////////////////////////////////////////////
    ///////////////////FIND_KEY//////////////////////////
    ////////////////////////////////////////////////////

    size_t find_key(const Key &key) const {
        // Börja på den position som key hashas till.
        size_t pos = hash_key(key);

        // Fortsätt söka så länge platsen är upptagen.
        while (used[pos]) {

            // Om vi hittar rätt key returnerar vi dess position.
            if (keys[pos] == key) {
                return pos;
            }

            // Om kollision uppstår går vi till nästa position.
            // % capacity gör att vi börjar om från 0 efter sista platsen.
            pos = (pos + 1) % capacity;
        }

        // Om platsen är tom innebär det att key inte finns i tabellen.
        return capacity;
    }
    /////////////////////////////////////////////////
    /////////////////////////////////////////////////

    // Erase a key from the hash table. Returns 'true' if a key was removed and
    // 'false' otherwise.
    bool erase_key(const Key &key) {
        // TODO: Finish the implementation. Return 'true' if the element was
        // found and removed. Otherwise, return 'false'. Remember to update
        // 'element_count'!
        return false;
    }

    // Allocate and initialize a new table.
    void alloc_table(size_t size) {
        capacity = size;
        element_count = 0;

        // Create arrays.
        used = new bool[capacity];
        keys = new Key[capacity];
        values = new Value[capacity];

        // Initialize 'used'.
        for (size_t i = 0; i < capacity; i++) {
            used[i] = false;
        }
    }

    // Grow the table if needed. Ensures that there are at least 2 free elements
    // in the table. This ensures that we have at least 1 free element after we
    // insert another element.
    void grow_if_needed() {
        // Grow if we have reached the load factor.
        bool need_grow = element_count >= capacity * max_load_factor;

        // Also make sure that we have at least one element that is free after
        // we insert another element.
        need_grow |= element_count + 2 >= capacity;

        // Check the load factor.
        if (need_grow) {
            grow();
        }
    }

    // Grow the hash table by doubling its size.
    void grow() {
        // Remember the old state.
        bool *old_used = used;
        Key *old_keys = keys;
        Value *old_values = values;
        size_t old_capacity = capacity;

        // Allocate a new table.
        alloc_table(capacity * 2);

        // Then just insert elements from "old" into the current table:
        for (size_t i = 0; i < old_capacity; i++) {
            if (!old_used[i])
                continue;

            insert_key(old_keys[i], old_values[i]);
        }

        // Finally, deallocate the old storage.
        delete[] old_used;
        delete[] old_keys;
        delete[] old_values;
    }

public:

    /**
     * Iterator. As long as you follow the interface outlined above, the
     * iterator should work as expected. For simplicity, we only implement const
     * iterators here. This is enough for what we need in the second part of the
     * lab, and to get the range-based for loops to work.
     */
    class const_iterator {
        friend class Hash_Map;

        // Remember our current position.
        const Hash_Map &m;
        size_t pos;

        // Create the iterator.
        const_iterator(const Hash_Map &m, size_t pos) : m{m}, pos{pos} {
            advance();
        }

        // Advance to the next used element.
        void advance() {
            while (pos < m.capacity && !m.used[pos]) {
                ++pos;
            }
        }

    public:
        // Compare to other iterators.
        bool operator ==(const const_iterator &o) const {
            return &m == &o.m && pos == o.pos;
        }

        bool operator !=(const const_iterator &o) const {
            return !(*this == o);
        }

        // Advance to the next element.
        const_iterator &operator ++() {
            if (pos < m.capacity)
                ++pos;
            advance();
            return *this;
        }

        // Prefix version of the ++ operator.
        const_iterator operator ++(int) {
            const_iterator tmp{*this};
            ++*this;
            return tmp;
        }

        // Get the current element.
        std::pair<Key, Value> operator *() const {
            return std::make_pair(m.keys[pos], m.values[pos]);
        }
    };

    // Create iterators.
    const_iterator begin() const {
        return const_iterator(*this, 0);
    }

    const_iterator end() const {
        return const_iterator(*this, capacity);
    }

    // Get the contents of a specific bucket. Mostly used for debugging. Returns
    // an empty optional if the element is empty.
    std::optional<std::pair<Key, Value>> bucket(size_t id) const {
        if (used[id] == true) {
            return std::make_pair(keys[id], values[id]);
        } else {
            return {};
        }
    }
};
#pragma once

#include <cstdint>
#include <memory>
#include <optional>
#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace inexor::vulkan_renderer {

/// @brief A manager class template for type managers.
/// In the Inexor engine, it is very common to have an unordered map of
/// key/value pairs for various data types. In most cases, std::string is the
/// key. The value however can be of an arbitrary data type. This template class
/// bundles common add/get/update/delete methods in a thread safe enviroment.
template <typename T>
class ManagerClassTemplate {
private:
    std::unordered_map<std::string, std::shared_ptr<T>> stored_types;

protected:
    // We use shared_mutex to give write access to exactly one thread, but read
    // access to all others. Shared mutexes are especially useful when shared data
    // can be safely read by any number of threads simultaneously, but a thread
    // may only write the same data when no other thread is reading or writing at
    // the same time.
    std::shared_mutex type_manager_shared_mutex;

    ManagerClassTemplate() = default;

    ~ManagerClassTemplate() = default;

    /// @brief Checks if a value exists by given key.
    /// @param type_name [in] The name of the type (the key).
    /// @return True if the value exists, false otherwise.
    bool does_key_exist(const std::string &type_name);

    // TODO: does_value_exist?

    /// @brief Adds a new type to the type map.
    /// @param type_name [in] The name of the new type (the key).
    /// @param new_type [in] The new type (the value).
    /// @note This method is thread safe thanks to the lock guard.
    /// @return True if adding the type was successful, false otherwise.
    bool add_entry(const std::string &type_name, const std::shared_ptr<T> new_type);

    /// @brief Updates the value of a type.
    /// @note This method is thread safe thanks to the lock guard.
    /// @param type_name [in] The name of the type (the key).
    /// @param new_type [in] The new type (the value).
    /// @note This method is thread safe thanks to the lock guard.
    /// @return True of the value could be updated, false if the key doesn't
    /// exist.
    bool update_entry(const std::string &type_name, const std::shared_ptr<T> new_type);

    /// @brief Returns a type (value) by given name (key).
    /// @param type_name [in] The name of the type (the key).
    /// @return An std::optional shared pointer of the type (the value).
    std::optional<std::shared_ptr<T>> get_entry(const std::string &type_name);

    /// @brief Returns the number of types available.
    /// @return The number of types available.
    std::size_t get_entry_count();

    /// @brief Returns all keys.
    /// @return A std::vector of shared pointers of the keys.
    /// TODO: Is there a way to ensure the data will be sent over as ready-only?
    /// We return a vector of shared pointers. This means the user could modify
    /// the manager class data without using the class mutex!
    std::vector<std::shared_ptr<T>> get_all_keys();

    /// @brief Returns all values.
    /// @return A std::vector of shared pointers of the values.
    /// TODO: Is there a way to ensure the data will be sent over as ready-only?
    /// We return a vector of shared pointers. This means the user could modify
    /// the manager class data without using the class mutex!
    std::vector<std::shared_ptr<T>> get_all_values();

    /// @brief Deletes a certain type by name (key).
    /// @param type_name [in] The name of the type to delete.
    /// @note This method is thread safe thanks to the lock guard.
    /// @return The number of deleted types.
    std::size_t delete_entry(const std::string &type_name);

    /// @brief Deletes all types.
    /// @TODO Refactor: Accept locked state so pre-shutdown doesn't have to unlock
    /// again before calling this method!
    /// @note This method is thread safe thanks to the lock guard.
    void delete_all_entries();
};

template <typename T>
bool ManagerClassTemplate<T>::does_key_exist(const std::string &type_name) {
    // Lock read access.
    std::shared_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    // Check if we can find the key.
    bool key_found = !(stored_types.end() == stored_types.find(type_name));

    return key_found;
}

template <typename T>
bool ManagerClassTemplate<T>::add_entry(const std::string &type_name, const std::shared_ptr<T> new_type) {
    if (does_key_exist(type_name)) {
        return false;
    }

    // Lock write access.
    std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    // Add a new entry.
    stored_types.insert({type_name, new_type});

    return true;
}

template <typename T>
bool ManagerClassTemplate<T>::update_entry(const std::string &type_name, const std::shared_ptr<T> new_type) {
    if (!does_key_exist(type_name)) {
        return false;
    }

    // Lock write access.
    std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    // Update the entry.
    stored_types[type_name] = new_type;

    return true;
}

template <typename T>
std::optional<std::shared_ptr<T>> ManagerClassTemplate<T>::get_entry(const std::string &type_name) {
    if (does_key_exist(type_name)) {
        // Lock read access.
        std::shared_lock<std::shared_mutex> lock(type_manager_shared_mutex);

        auto return_value = stored_types[type_name];

        return return_value;
    }

    return std::nullopt;
}

template <typename T>
std::size_t ManagerClassTemplate<T>::get_entry_count() {
    // Lock read access.
    std::shared_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    std::size_t map_size = stored_types.size();

    return map_size;
}

template <typename T>
std::vector<std::shared_ptr<T>> ManagerClassTemplate<T>::get_all_keys() {
    // get_entry_count() will lock read access automatically for us.
    std::size_t map_size = get_entry_count();

    std::vector<std::shared_ptr<T>> all_keys;

    if (0 == map_size) {
        return all_keys;
    }

    all_keys.reserve(stored_types.size());

    // Lock read access.
    std::shared_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    // Iterate through map and fill values into vector.
    for (auto it = stored_types.begin(); it != stored_types.end(); ++it) {
        all_keys.push_back(it->first);
    }

    return all_keys;
}

template <typename T>
std::vector<std::shared_ptr<T>> ManagerClassTemplate<T>::get_all_values() {
    std::vector<std::shared_ptr<T>> all_values;

    // get_entry_count() will lock read access automatically for us.
    std::size_t map_size = get_entry_count();

    if (0 == map_size) {
        return all_values;
    }

    all_values.reserve(stored_types.size());

    // Lock read access.
    std::shared_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    // Iterate through map and fill values into vector.
    for (auto it = stored_types.begin(); it != stored_types.end(); ++it) {
        all_values.push_back(it->second);
    }

    return all_values;
}

template <typename T>
std::size_t ManagerClassTemplate<T>::delete_entry(const std::string &type_name) {
    if (!does_key_exist(type_name)) {
        return 0;
    }

    // Lock write access.
    std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    std::size_t number_of_deleted_entries = stored_types.erase(type_name);

    return number_of_deleted_entries;
}

template <typename T>
void ManagerClassTemplate<T>::delete_all_entries() {
    // Lock write access.
    std::unique_lock<std::shared_mutex> lock(type_manager_shared_mutex);

    stored_types.clear();
}

} // namespace inexor::vulkan_renderer

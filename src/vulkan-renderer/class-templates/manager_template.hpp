#pragma once

#include <unordered_map>
#include <optional>
#include <mutex>


namespace inexor {
namespace vulkan_renderer {


	/// @class ManagerClassTemplate
	/// @brief A manager class template for type managers.
	/// In the Inexor engine, it is very common to have an unordered map of key/value pairs
	/// for various data types. In most cases, std::string is the key. The value however can
	/// be of an arbitrary data type. This template class bundles common add/get/update/delete
	/// methods in a thread safe enviroment.
	template <typename T>
	class ManagerClassTemplate
	{
		private:
			
			std::unordered_map<std::string, std::shared_ptr<T>> stored_types;

			std::mutex type_manager_lock;


		protected:


			ManagerClassTemplate() = default;
			
			~ManagerClassTemplate() = default;


			/// @brief Checks if a value exists by given key.
			/// @param type_name [in] The name of the type (the key).
			/// @return True if the value exists, false otherwise.
			bool does_type_exist(const std::string& type_name)
			{
				return !(stored_types.end() == stored_types.find(type_name));
			}



			/// @brief Adds a new type to the type map.
			/// @param type_name [in] The name of the new type (the key).
			/// @param new_type [in] The new type (the value).
			/// @note This method is thread safe thanks to the lock guard.
			/// @return True if adding the type was successful, false otherwise.
			bool add_type(const std::string& type_name, const std::shared_ptr<T> new_type)
			{
				if(does_type_exist(type_name))
				{
					return false;
				}

				// Use lock guard to ensure thread safety.
				std::lock_guard<std::mutex> lock(type_manager_lock);

				// Store the new type.
				stored_types.insert({type_name, new_type});

				return true;
			}


			/// @brief Updates the value of a type.
			/// @note This method is thread safe thanks to the lock guard.
			/// @param type_name [in] The name of the type (the key).
			/// @param new_type [in] The new type (the value).
			/// @note This method is thread safe thanks to the lock guard.
			/// @return True of the value could be updated, false if the key doesn't exist.
			bool update_type(const std::string& type_name, const std::shared_ptr<T> new_type)
			{
				if(!does_type_exist(type_name))
				{
					return false;
				}
				
				// Use lock guard to ensure thread safety.
				std::lock_guard<std::mutex> lock(type_manager_lock);

				// Update the entry.
				stored_types[type_name] = new_type;

				return true;
			}


			/// @brief Returns a type (value) by given name (key).
			/// @param type_name [in] The name of the type (the key).
			/// @return An std::optional shared pointer of the type (the value).
			std::optional<std::shared_ptr<T>> get_type(const std::string& type_name)
			{
				if(does_type_exist(type_name))
				{
					// No mutex required as this is a read only operation.
					return stored_types[type_name];
				}

				return std::nullopt;
			}


			/// @brief Returns the number of types available.
			/// @return The number of types available.
			std::size_t get_type_count() const
			{
				// No mutex required as this is a read only operation.
				return stored_types.size();
			}


			/// @brief Returns all types.
			/// @return A std::vector of shared pointers of types (values).
			std::vector<std::shared_ptr<T>> get_all_types() const
			{
				std::vector<std::shared_ptr<T>> all_types;

				if(0 == stored_types.size())
				{
					return all_types;
				}

				all_types.reserve(stored_types.size());

				// Iterate through map and fill values into vector.
				for(auto it = stored_types.begin(); it != stored_types.end(); ++it)
				{
					all_types.push_back(it->second);
				}

				return all_types;
			}


			/// @brief Deletes a certain type by name (key).
			/// @param type_name [in] The name of the type to delete.
			/// @note This method is thread safe thanks to the lock guard.
			/// @return The number of deleted types.
			std::size_t delete_type(const std::string& type_name)
			{
				if(!does_type_exist(type_name))
				{
					return 0;
				}
				
				// Use lock guard to ensure thread safety.
				std::lock_guard<std::mutex> lock(type_manager_lock);
				
				std::size_t number_of_deleted_entries = stored_types.erase(type_name);

				return number_of_deleted_entries;
			}

			
			/// @brief Deletes all types.
			/// @note This method is thread safe thanks to the lock guard.
			void delete_all_types()
			{
				// Use lock guard to ensure thread safety.
				std::lock_guard<std::mutex> lock(type_manager_lock);

				// Use lock guard to ensure thread safety.
				stored_types.clear();
			}


	};


};
};

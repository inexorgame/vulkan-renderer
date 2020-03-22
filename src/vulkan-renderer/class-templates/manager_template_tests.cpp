#include <gtest/gtest.h>
#include <spdlog/spdlog.h>

#include <memory>
#include <string>

#include "manager_template.hpp"


TEST(manager_template_class, does_type_exist)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;

	bool type_lookup = manager.does_type_exist("example_type_1");

	EXPECT_EQ(type_lookup, false);

	std::shared_ptr<std::string> type1 = std::make_shared<std::string>();

	manager.add_type("example_type_1", type1);
	
	type_lookup = manager.does_type_exist("example_type_1");

	EXPECT_EQ(type_lookup, true);

	std::size_t number_of_types = manager.get_type_count();
	
	manager.delete_all_types();
	
	std::size_t number_of_types2 = manager.get_type_count();

	EXPECT_NE(number_of_types, number_of_types2);
	EXPECT_EQ(number_of_types2, 0);
}


TEST(manager_template_class, add_type)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	std::shared_ptr<std::string> type1 = std::make_shared<std::string>();

	bool succeeded = manager.add_type("example_type_1", type1);

	EXPECT_EQ(succeeded, true);
	
	// This must fail as the key already exists!
	succeeded = manager.add_type("example_type_1", type1);

	EXPECT_EQ(succeeded, false);

	std::size_t number_of_types = manager.get_type_count();
	
	manager.delete_all_types();
	
	std::size_t number_of_types2 = manager.get_type_count();

	EXPECT_NE(number_of_types, number_of_types2);
	EXPECT_EQ(number_of_types2, 0);
}


TEST(manager_template_class, update_existing_type)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	std::shared_ptr<std::string> type1 = std::make_shared<std::string>("This is an example text.");

	manager.add_type("type1", type1);

	std::shared_ptr<std::string> type2 = std::make_shared<std::string>("Yet another sentence.");

	// This will fail because the key doesn't exist yet.
	// Keys will not be created automatically.
	bool update_result = manager.update_type("type2", type2);

	EXPECT_EQ(update_result, false);

	// Update an existing type.
	manager.update_type("type1", type2);

	// Get the current value by key.
	std::optional<std::shared_ptr<std::string>> type1_lookup = manager.get_type("type1");

	bool is_nullopt = (std::nullopt == type1_lookup);

	EXPECT_EQ(is_nullopt, false);

	std::string type1_str_value = *type1_lookup.value();

	bool are_strings_equal = (0 == type1->compare(type1_str_value));

	EXPECT_EQ(are_strings_equal, false);

	std::size_t number_of_types = manager.get_type_count();
	
	manager.delete_all_types();
	
	std::size_t number_of_types2 = manager.get_type_count();

	EXPECT_NE(number_of_types, number_of_types2);
	EXPECT_EQ(number_of_types2, 0);
}


TEST(manager_template_class, delete_all_types)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	std::shared_ptr<std::string> type1 = std::make_shared<std::string>("This is another weird example text.");

	manager.add_type("type1", type1);

	std::size_t number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 1);

	manager.delete_all_types();

	number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 0);
}


TEST(manager_template_class, delete_type)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	std::shared_ptr<std::string> type1 = std::make_shared<std::string>("Ok seriously stop it.");

	manager.add_type("type1", type1);

	std::size_t number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 1);

	manager.delete_type("type1");
	
	number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 0);
}


TEST(manager_template_class, get_type)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	std::shared_ptr<std::string> type1 = std::make_shared<std::string>("Ok seriously stop it.");

	manager.add_type("type1", type1);

	std::size_t number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 1);

	auto type1_lookup = manager.get_type("type1");
	
	bool is_equal = (0 == type1_lookup.value()->compare("Ok seriously stop it."));

	EXPECT_EQ(is_equal, true);

	manager.delete_all_types();

	number_of_types = manager.get_type_count();
	
	EXPECT_EQ(number_of_types, 0);
}


TEST(manager_template_class, get_all_types)
{
	using namespace inexor::vulkan_renderer;

	ManagerClassTemplate<std::string> manager;
	
	const std::vector<std::string> example_texts =
	{
		"This is a test",
		"This is another test",
		"I'm not good at inventing example sentences",
		"The quick brown duck or whatever jumps in circles around the oceans?",
	};

	std::vector<std::shared_ptr<std::string>> example_types;

	std::size_t index = 0;

	for(const auto& example : example_texts)
	{
		std::string key_name = "index_"+ std::to_string(index);

		// Create a new value.
		std::shared_ptr<std::string> new_type = std::make_shared<std::string>(example_texts[index]);

		example_types.push_back(new_type);

		manager.add_type(key_name, example_types[index]);

		index++;
	}

	std::size_t number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, example_texts.size());

	std::vector<std::shared_ptr<std::string>> get_all = manager.get_all_types();

	index = 0;

	for(const auto& value : get_all)
	{
		bool are_strings_equal = (0 == value->compare(example_texts[index]));

		EXPECT_EQ(are_strings_equal, true);

		index++;
	}

	manager.delete_all_types();

	number_of_types = manager.get_type_count();

	EXPECT_EQ(number_of_types, 0);
}

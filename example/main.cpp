#include "inexor/vulkan-renderer/application.hpp"

#include "inexor/vulkan-renderer/tools/allocators/pool_allocator.hpp"

#include <stdexcept>
#include <iostream>

int main(int argc, char *argv[]) {

    class Person {
    public:
        int m_age;
        std::string m_name;

        Person(int age, std::string name) : m_age(age), m_name(name) {}

        void print() {
            std::cout << m_name << " " << m_age << std::endl;
        }

    };

    inexor::vulkan_renderer::tools::allocators::PoolAllocator<Person> persons;

    auto p1 = persons.allocate(18, "Johannes");
    auto p2 = persons.allocate(17, "Dieter");
    auto p3 = persons.allocate(143, "Sabine");

    p1->print();
    p2->print();
    p3->print();

    persons.deallocate(p1);
    persons.deallocate(p2);
    persons.deallocate(p3);

    try {
        using inexor::vulkan_renderer::Application;
        auto renderer = std::make_unique<Application>(argc, argv);
        renderer->run();
    } catch (const std::exception &exception) {
        spdlog::critical(exception.what());
        return 1;
    }
    return 0;
}

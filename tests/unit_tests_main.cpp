#include <gtest/gtest.h>

#include <iostream>

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    std::cout << "RUN_ALL_TESTS() returned: " << RUN_ALL_TESTS() << std::endl;
    std::cin.get();
}

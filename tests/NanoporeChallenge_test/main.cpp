#include <iostream>
#include <gtest/gtest.h>

TEST(Addition, AddTwoNumbers) {
    ASSERT_EQ(2+2, 4) << "Numbers are not equal";

}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

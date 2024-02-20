#ifdef TESTING

#include "prefix.hpp"

#include "doctest.h"

TEST_CASE("non_neg_prefix_sum")
{
    SUBCASE("correctly returns true")
    {
        const auto data = {1, -1, 1, -1};
        CHECK(non_neg_prefix_sum(data));
    }

    SUBCASE("correctly returns false")
    {
        const auto data = {1, -1, -1, 1};
        CHECK_FALSE(non_neg_prefix_sum(data));
    }

    SUBCASE("handles zero-length input")
    {
        const std::vector<int> data;
        CHECK(non_neg_prefix_sum(data));
    }

    SUBCASE("handles long input")
    {
        // create a long sequence of {1,-1,1,...}
        std::vector<int> data;
        const int n[2]{1, -1};
        for (size_t i = 0; i < 1 << 20; ++i) {
            data.push_back(n[i % 2]);
        }
        CHECK(non_neg_prefix_sum(data));
        // check for false as well
        std::swap(data[1 << 10], data[(1 << 10) + 1]);
        CHECK_FALSE(non_neg_prefix_sum(data));
    }
}

TEST_CASE("non_pos_prefix_sum")
{
    SUBCASE("correctly returns true")
    {
        const auto data = {-1, 1, -1, 1};
        CHECK(non_pos_prefix_sum(data));
    }

    SUBCASE("correctly returns false")
    {
        const auto data = {1, -1, -1, 1};
        CHECK_FALSE(non_pos_prefix_sum(data));
    }

    SUBCASE("handles zero-length input")
    {
        const std::vector<int> data;
        CHECK(non_pos_prefix_sum(data));
    }

    SUBCASE("handles long input")
    {
        // create a long sequence of {-1,1,-1,...}
        std::vector<int> data;
        const int n[2]{-1, 1};
        for (size_t i = 0; i < 1 << 20; ++i) {
            data.push_back(n[i % 2]);
        }
        CHECK(non_pos_prefix_sum(data));
        // check for false as well
        std::swap(data[1 << 10], data[(1 << 10) + 1]);
        CHECK_FALSE(non_pos_prefix_sum(data));
    }
}

#endif

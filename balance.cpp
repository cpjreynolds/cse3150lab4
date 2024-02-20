#ifdef TESTING
#include "doctest.h"
#include "balance.hpp"

// the RNG seed is always the same during testing so these are replicable.

TEST_CASE("symbols class")
{

    SUBCASE("do_scramble")
    {
        SUBCASE("n=3")
        {
            symbols data1(3);
            symbols result1 = {1, 1, -1, -1, -1, 1, -1};

            data1.scramble();
            CHECK_EQ(data1, result1);
        }
        SUBCASE("n=8") {}
    }

    SUBCASE("cut_and_paste")
    {
        SUBCASE("n=3")
        {
            symbols data(3);
            symbols result = {1, 1, -1, -1, 1, -1};
            data.scramble();
            data.cut_and_splice();
            CHECK_EQ(data, result);
        }

        SUBCASE("n=8")
        {
            symbols data(8);
            symbols result = {1, 1, -1, -1, 1, -1};
            data.scramble();
            data.cut_and_splice();
            CHECK_EQ(data, result);
        }
    }

    SUBCASE("is_balanced")
    {
        SUBCASE("n=3")
        {
            symbols data(3);
            data.scramble();
            data.cut_and_splice();
            CHECK(data.is_balanced());
        }
        SUBCASE("n=8")
        {
            symbols data(8);
            data.scramble();
            data.cut_and_splice();
            CHECK(data.is_balanced());
        }
    }
}

#endif

#ifndef PREFIX_HPP
#define PREFIX_HPP

#include <ranges>

/*
 * takes any integral range and tests its prefix sums for non-negativity
 */
template<std::ranges::input_range R>
    requires std::integral<std::ranges::range_value_t<R>>
bool non_neg_prefix_sum(const R& r)
{
    using Int = std::ranges::range_value_t<R>;
    Int sum = 0;
    for (const Int& i : r) {
        sum += i;
        if (sum < 0) {
            return false;
        }
    }
    return true;
}

/*
 * Takes any integer iterable and tests its prefix sums for non-positivity
 */
template<std::ranges::input_range R>
    requires std::integral<std::ranges::range_value_t<R>>
bool non_pos_prefix_sum(const R& r)
{
    using Int = std::ranges::range_value_t<R>;
    Int sum = 0;
    for (const Int& i : r) {
        sum += i;
        if (sum > 0) {
            return false;
        }
    }
    return true;
}

#endif

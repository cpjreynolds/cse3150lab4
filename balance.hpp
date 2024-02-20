#ifndef BALANCE_HPP
#define BALANCE_HPP

#include <algorithm>
#include <iterator>
#include <vector>
#include <iostream>
#include <random>
#include <numeric>
#include <functional>

#include "prefix.hpp"

// represents a list of symbols.
//
// could have templated on symbol type but it made list-init a pain.
class symbols : public std::vector<int8_t> {
public:
    using std::vector<int8_t>::vector;

    // create a list of symbols of `n` 1s and `n+1` -1s
    explicit symbols(size_t n) : vector(2 * n + 1)
    {
        auto b = begin();
        auto h = b + n;
        auto e = end();
        std::fill(b, h, 1);
        std::fill(h, e, -1);
    }

    // true if symbols have a non-negative prefix sum
    //
    // (per the assignment sheet ONLY non-neg counts, not non-neg OR non-pos)
    bool is_balanced() const { return non_neg_prefix_sum(*this); }

    // performs the in-place Fisher-Yates scramble on the symbols
    //
    // uses c++ random facilities as `std::rand() % range` introduces
    // statistical bias whereas `uniform_int_distribution` does not.
    //
    // `bias` = true will bias the results for use in testing.
    void scramble(bool bias = false)
    {
        auto dist = [=](size_t a, auto& rd) {
            if (!bias) {
                return std::uniform_int_distribution<size_t>(0, a)(rd);
            }
            else {
                return std::binomial_distribution<size_t>(a, 0.5)(rd);
            }
        };
        for (size_t i = size() - 1; i > 0; --i) {
            auto n = dist(i, rd);
            std::swap((*this)[i], (*this)[n]);
        }
    }

    // returns a const_iterator to the lowest valley
    vector::const_iterator lowest_valley() const
    {
        // int because the sums could be larger than int8_t
        std::vector<int> sums;
        std::partial_sum(cbegin(), cend(), std::back_inserter(sums));
        auto m = std::min_element(sums.cbegin(), sums.cend());
        return cbegin() + std::distance(sums.cbegin(), m);
    }

    // performs the [P2:P1'] splicing from the assignment algorithm.
    void cut_and_splice()
    {
        auto i = lowest_valley();
        // exclude i itself as thats the final -1 edge
        std::vector<int8_t> p2(i + 1, cend());
        p2.insert(p2.end(), cbegin(), i);
        vector::operator=(std::move(p2));
    }

    // generate `nsyms` symbols of size `2n+1`
    static std::vector<symbols> generate_n(size_t n, size_t nsyms)
    {
        const symbols sym(n);
        std::vector<symbols> syms(nsyms, sym);
        return syms;
    }

    // returns the highest and lowest values of the partial sums.
    std::pair<int, int> hilo() const
    {
        std::vector<int> sums;
        std::partial_sum(cbegin(), cend(), std::back_inserter(sums));
        int high = *std::max_element(sums.cbegin(), sums.cend());
        high = high < 0 ? 0 : high;
        int low = *std::min_element(sums.cbegin(), sums.cend());
        low = low > 0 ? 0 : low;

        return {high, low};
    }

    // this assignment was pretty easy so as a fun challenge I wrote this to
    // print the symbols like the graph from the assignment sheet.
    //
    // returns the string representation of the graph of the symbol sequence
    std::string graph() const
    {
        auto [high, low] = hilo();

        // create a 2d array of spaces holding the max extent of the graph.
        // plus 4 extra spaces for the surrounding '_'s that show y=0
        std::vector<std::string> lines(high - low,
                                       std::string(size() + 4, ' '));

        int lidx = -low; // current line
        int cidx = 2;    // current column
        for (const auto& i : *this) {
            if (i == 1) {
                lines[lidx][cidx] = '/';
                lidx++;
            }
            else { // i == -1
                --lidx;
                lines[lidx][cidx] = '\\';
            }
            ++cidx;
        }

        // add some indication of y=0
        if (-low == lines.size()) {
            lines.push_back(std::string(size() + 4, '_'));
        }
        else {
            auto& mid = lines[-low];
            std::for_each(mid.begin(), mid.end(),
                          [](auto& c) { c == ' ' ? c = '_' : 0; });
        }

        std::string graph;
        for (const auto& line : lines | std::views::reverse) {
            graph += line;
            graph += '\n';
        }
        return graph;
    }

    friend std::ostream& operator<<(std::ostream& os, const symbols& s)
    {
        if (s.empty()) {
            return os << "{}";
        }
        // at least size() == 1
        os << "{";
        auto it = s.cbegin();
        auto end = s.cend() - 1;

        while (it != end) {
            // deliberate postincrement
            // static_cast required to print int8_t as integer
            os << static_cast<int>(*it++) << ", ";
        }
        os << static_cast<int>(*it) << "}";
        return os;
    }

    friend std::hash<symbols>;

private:
    // returns a string representation of the sequence appropriate for hashing
    std::string to_string() const
    {
        std::string repr;
        repr.reserve(size());
        std::ranges::transform(
            *this, std::back_inserter(repr),
            [](const int8_t& x) { return x == 1 ? '1' : '0'; });
        return repr;
    }

    // returns a uint64_t suitable for hashing. used when size() < 64
    size_t to_bits() const
    {
        size_t bits = 0;
        size_t i;
        for (i = 0; i < size() - 1; ++i) {
            bits |= (*this)[i] == 1 ? 1 : 0;
            bits <<= 1;
        }
        bits |= (*this)[i] == 1 ? 1 : 0;
        return bits;
    }

    // static so the RNG is instantiated only once upon first invocation.
#ifdef TESTING
    // use the default seed to get reproducible tests
    inline static std::mt19937 rd{};
#else
    // otherwise get actual entropy from the hardware
    inline static std::mt19937 rd{std::random_device{}()};
#endif
};

template<>
struct std::hash<symbols> {
    size_t operator()(const symbols& s) const noexcept
    {
        if (s.size() > sizeof(size_t) * 8) {
            return std::hash<std::string>{}(s.to_string());
        }
        else {
            return std::hash<size_t>{}(s.to_bits());
        }
    }
};

#endif

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iterator>
#include <numeric>
#include <random>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include "balance.hpp"

template<std::ranges::input_range R>
    requires std::integral<std::ranges::range_value_t<R>> ||
             std::floating_point<std::ranges::range_value_t<R>>
static double variance(const R& lst)
{
    double mean = std::accumulate(lst.begin(), lst.end(), 0.0) / lst.size();
    std::vector<double> sqdiffs;
    std::ranges::transform(lst, std::back_inserter(sqdiffs), [=](auto& n) {
        return std::pow(double(n) - mean, 2);
    });
    double variance = std::accumulate(sqdiffs.begin(), sqdiffs.end(), 0.0) /
                      (sqdiffs.size() - 1);
    return variance;
}

/*
 * returns the standard deviation of the given list.
 *
 * used to determine convergence.
 *
 * can take integral or floating point ranges but always returns double
 */
template<std::ranges::input_range R>
    requires std::integral<std::ranges::range_value_t<R>> ||
             std::floating_point<std::ranges::range_value_t<R>>
static double stddev(const R& lst)
{
    return std::sqrt(variance(lst));
}

// generates `ns` symbols of size `2n+1`, scrambles them, balances them, and
// populates the `table` with the unique balanced lists and their respective
// number of occurences.
//
// biases the scramble function if `bias == true` (for testing)
//
// returns the standard deviation of the frequencies of each unique balanced
// list and the total number of symbols tested.
static std::pair<double, int>
run_iteration(std::unordered_map<symbols, int>& table, size_t n, size_t ns,
              bool bias = false)
{
    std::vector<symbols> syms = symbols::generate_n(n, ns);
    std::ranges::for_each(syms, [=](auto& s) {
        s.scramble(bias);
        s.cut_and_splice();
    });
    std::ranges::for_each(syms, [&](auto& s) {
        auto [kv, ins] = table.insert({s, 1});
        if (!ins) {
            auto& [_, v] = *kv;
            v += 1;
        }
    });
    auto vals = std::views::values(table);
    int nsyms = std::accumulate(vals.begin(), vals.end(), 0);

    std::vector<double> freqs;
    std::ranges::transform(vals, std::back_inserter(freqs),
                           [=](auto& v) { return v / double(nsyms); });

    return {stddev(freqs), nsyms};
}

// calls `run_iteration(table, n, ns)` until the distribution of unique balanced
// lists has been shown to be uniform.
//
// uniformity determined by `stddev(freq_of_unique_lists) < (1/n_unique)*eps`
//
// throws an exception if distribution does not converge within `max_iters`
// iterations.
//
// **NOTE**: n > 10 has extremely long runtime and likely will not terminate
static std::pair<double, int>
run_to_convergence(std::unordered_map<symbols, int>& table, size_t n, size_t ns,
                   double eps, size_t max_iters, bool bias = false)
{
    double sdev;
    int nsyms;
    size_t iters = 0;

    do {
        std::tie(sdev, nsyms) = run_iteration(table, n, ns, bias);
        ++iters;
        if (++iters > max_iters) {
            throw std::runtime_error("maximum iterations");
        }
        std::cout << sdev << '\t' << eps * (1.0 / table.size()) << std::endl;
    } while (sdev > eps * (1.0 / table.size()) || iters < n * 2);

    return {sdev, nsyms};
}

#ifndef TESTING

// mush 2 graphs together on the same lines for output
//
// this took longer to get correct than literally the entire rest of the lab.
//
// string manipulation is hazardous to sane programmers' health.
//
// tried to get it done with as few copies as possible.
static std::string merge2(const std::string& lhs, const std::string& rhs)
{
    std::vector<std::string_view> leftlines, rightlines;
    // split returns a non-owning subrange
    for (auto l : lhs | std::views::split('\n')) {
        if (!l.empty()) {
            leftlines.push_back(std::string_view(l.data(), l.size()));
        }
    }
    for (auto l : rhs | std::views::split('\n')) {
        if (!l.empty()) {
            rightlines.push_back(std::string_view(l.data(), l.size()));
        }
    }

    size_t nleft = leftlines.size();
    size_t nright = rightlines.size();

    // lifetime needs to extend to the end of the function so the str_views
    // are valid.
    std::string filler;
    if (nleft > nright) {
        filler = std::string(rightlines[0].size(), ' ');
        rightlines.insert(rightlines.begin(), nleft - nright, filler);
    }
    else if (nright > nleft) {
        filler = std::string(leftlines[0].size(), ' ');
        leftlines.insert(leftlines.begin(), nright - nleft, filler);
    }

    std::ostringstream output;
    for (size_t li = 0; li < leftlines.size() && li < rightlines.size(); ++li) {
        output << leftlines[li] << '|' << rightlines[li] << '\n';
    }
    return output.str();
}

// create the output string of graphs all merged together in rows with `cols`
// columns.
static std::string paste_graphs(std::vector<std::pair<symbols, int>>& lst,
                                int cols)
{
    std::ostringstream output;

    int row = 0;
    for (; row < lst.size() / cols; ++row) {
        std::string line = lst[row * cols].first.graph();
        for (int col = 1; col < cols; ++col) {
            line = merge2(line, lst[(row * cols) + col].first.graph());
        }
        output << line << '\n';
    }
    // leftover
    if (row * cols < lst.size()) {
        std::string line = lst[row * cols].first.graph();
        for (int i = row * cols + 1; i < lst.size(); ++i) {
            line = merge2(line, lst[i].first.graph());
        }
        output << line;
    }
    std::string s = output.str();
    if (*(s.end() - 2) == '\n') {
        // remove an extra newline;
        s.pop_back();
    }
    return s;
}

// prints a random selection of `n` graphs from the given table of lists.
static void print_selection(std::unordered_map<symbols, int>& table, int n)
{
    std::vector<std::pair<symbols, int>> out;
    auto gen = std::mt19937{std::random_device{}()};
    std::ranges::sample(table, std::back_inserter(out), n, gen);
    int maxwide = 80;                           // 80 columns is pretty standard
    int wide = table.begin()->first.size() + 4; // 4 padding spaces
    auto s = paste_graphs(out, std::max(maxwide / wide, 1));
    std::cout << s << std::endl;
}

constexpr size_t DEFAULT_NSYMS = 1 << 16;
constexpr size_t DEFAULT_N = 4;
constexpr double DEFAULT_EPS = 0.1;
constexpr size_t DEFAULT_MAXITERS = 1 << 10;

constexpr std::string_view USAGE =
    "USAGE: ./lab4.out [n=4] [nsyms=65536] [maxiters=1024] [eps=0.1]\n";

int main(int argc, char** argv)
{
    size_t nsyms = DEFAULT_NSYMS;
    size_t n = DEFAULT_N;
    size_t maxi = DEFAULT_MAXITERS;
    double eps = DEFAULT_EPS;

    try {
        if (argc > 1) {
            n = std::stoul(argv[1]);
        }
        if (argc > 2) {
            nsyms = std::stoul(argv[2]);
        }
        if (argc > 3) {
            maxi = std::stoul(argv[3]);
        }
        if (argc > 4) {
            eps = std::stod(argv[3]);
        }
        if (argc > 5) {
            throw std::runtime_error("invalid number of arguments");
        }
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << "\n\n";
        std::cerr << USAGE;
        return 1;
    }

    std::unordered_map<symbols, int> table;

    std::cout << std::fixed;
    try {
        double sd;
        int ns;
        if (n <= 10) {
            std::tie(sd, ns) = run_to_convergence(table, n, nsyms, eps, maxi);
            std::cout << "convergence for ";
            std::cout << "(n=" << n << ", nsyms=" << nsyms << ", eps=" << eps
                      << ")"
                      << ":\n";
            std::cout << "unique lists\t= " << table.size() << std::endl;
            std::cout << "total samples\t= " << ns << std::endl;
            std::cout << "uniform freq.\t= " << (1.0 / table.size())
                      << std::endl;
            std::cout << "stddev(freqs)\t= " << sd << std::endl;
        }
        else { // n is too great for convergence in an acceptable timeframe
            std::tie(std::ignore, ns) = run_iteration(table, n, nsyms);
            size_t uniq = table.size();
            std::tie(std::ignore, ns) = run_iteration(table, n, nsyms);
            size_t nuniq = table.size();
            while (uniq != nuniq) {
                // find how many unique lists there are at least
                std::tie(std::ignore, ns) = run_iteration(table, n, nsyms);
                uniq = nuniq;
                nuniq = table.size();
            }
        }
        std::cout << "result for ";
        std::cout << "(n=" << n << ", nsyms=" << nsyms << ")"
                  << ":\n";
        std::cout << "unique lists\t= " << table.size() << std::endl;
        std::cout << "total samples\t= " << ns << std::endl;

        // literally just because I was bored and wanted an excuse to do
        // more programming.
        //
        // prints up to 20 of the unique balanced lists to stdout, chosen
        // randomly.
        int nprint = table.size() < 20 ? table.size() : 20;
        std::cout << "\n(" << nprint << "/" << table.size()
                  << ") unique lists:\n\n";
        print_selection(table, nprint);
    }
    catch (const std::exception& e) {
        std::cout << "distribution did not converge after " << maxi
                  << " iterations" << std::endl;
    }

    return 0;
}

#else
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

TEST_CASE("stddev")
{
    SUBCASE("stddev(double)")
    {
        std::vector<double> data1{1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::vector<double> data2{data1};
        std::reverse(data2.begin(), data2.end());
        std::vector<double> data3(100);
        std::iota(data3.begin(), data3.end(), 1);

        // checked with Mathematica
        CHECK_EQ(stddev(data1), doctest::Approx(2.7386));
        CHECK_EQ(stddev(data2), doctest::Approx(2.7386));
        CHECK_EQ(stddev(data3), doctest::Approx(29.0115));
    }

    SUBCASE("stddev(int)")
    {
        // ensuring the templating worked properly
        std::vector<int> data1{1, 2, 3, 4, 5, 6, 7, 8, 9};
        std::vector<int> data2{data1};
        std::reverse(data2.begin(), data2.end());
        std::vector<int> data3(100);
        std::iota(data3.begin(), data3.end(), 1);

        CHECK_EQ(stddev(data1), doctest::Approx(2.7386));
        CHECK_EQ(stddev(data2), doctest::Approx(2.7386));
        CHECK_EQ(stddev(data3), doctest::Approx(29.0115));
    }
}

#ifdef FULLCHECK // these tests are slow so conditionally compile
TEST_CASE("n=4")
{
    size_t n = 4;
    size_t ns = 1 << 16;
    double eps = 0.1;
    size_t maxi = 50;
    std::unordered_map<symbols, int> table;
    SUBCASE("convergence")
    {
        // just returning demonstrates convergence.
        CHECK_NOTHROW(run_to_convergence(table, n, ns, eps, maxi));
    }
    SUBCASE("biased non-convergence")
    {
        CHECK_THROWS(run_to_convergence(table, n, ns, eps, maxi, true));
    }
}

TEST_CASE("n=10")
{
    size_t n = 10;
    size_t ns = 1 << 16;
    double eps = 0.1;
    size_t maxi = 75;
    std::unordered_map<symbols, int> table;
    SUBCASE("convergence")
    {
        CHECK_NOTHROW(run_to_convergence(table, n, ns, eps, maxi));
    }
    SUBCASE("biased non-convergence")
    {
        CHECK_THROWS(run_to_convergence(table, n, ns, eps, maxi, true));
    }
}
#endif
#endif

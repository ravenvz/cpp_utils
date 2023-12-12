#ifndef STRING_EXT_H_0CNTGIMQ
#define STRING_EXT_H_0CNTGIMQ

#include "cpp_utils/algorithms/ranges_ext.h"
#include <functional>
#include <iomanip>
#include <list>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace alg {

template <typename T>
concept might_be_empty = requires(const T& val)
{
    val.empty();
};

template <typename Iterator>
concept input_iterator_to_emptiable_type = requires
{
    requires std::input_iterator<Iterator>;
    requires might_be_empty<
        typename std::iterator_traits<Iterator>::value_type>;
};

template <typename Iterator>
concept input_iterator_to_non_emptiable_type = requires
{
    requires std::input_iterator<Iterator>;
    requires not might_be_empty<
        typename std::iterator_traits<Iterator>::value_type>;
};

struct join_fn {

    template <input_iterator_to_non_emptiable_type I,
              std::sentinel_for<I> S,
              typename Proj = std::identity>
    auto operator()(I first,
                    S last,
                    const std::string& delimeter,
                    Proj proj = {}) const -> std::string
    {
        std::ostringstream res;
        for (auto it = first; it != last; ++it) {
            res << std::invoke(proj, *it);
            if (std::distance(it, last) > 1)
                res << delimeter;
        }
        return res.str();
    }

    // Skips values where holds val.empty() == true
    template <input_iterator_to_emptiable_type I,
              std::sentinel_for<I> S,
              typename Proj = std::identity>
    auto operator()(I first,
                    S last,
                    const std::string& delimeter,
                    Proj proj = {}) const -> std::string
    {
        std::ostringstream res;
        for (; first != last; ++first) {
            res << std::invoke(proj, *first);
            if (!first->empty() && std::distance(first, last) > 1)
                res << delimeter;
        }
        return res.str();
    }

    template <std::ranges::input_range R, typename Proj = std::identity>
    auto operator()(R&& r, const std::string& delimeter, Proj proj = {}) const
        -> std::string
    {
        return (*this)(std::ranges::begin(r),
                       std::ranges::end(r),
                       delimeter,
                       std::ref(proj));
    }
};

inline constexpr join_fn join;

/* Given a text as string, populate provided iterator with all words in
   text. Words are defined by expr regex. Default regex allow words to
   contain letters, digits, '+', '-'. */
struct parse_words_fn {

    template <std::input_iterator I,
              std::sentinel_for<I> S,
              std::weakly_incrementable Out>
    auto operator()(I first,
                    S last,
                    Out out,
                    std::regex expr = std::regex{"[[:alnum:]+-]+"}) const -> Out
    {
        std::sregex_iterator words_begin{first, last, expr};
        std::sregex_iterator words_end;

        for (auto it = words_begin; it != words_end; ++it) {
            *out++ = it->str();
        }

        return out;
    }

    template <std::ranges::input_range R, std::weakly_incrementable Out>
    auto operator()(R&& r,
                    Out out,
                    std::regex expr = std::regex{"[[:alnum:]+-]+"}) const -> Out
    {
        return (*this)(std::ranges::begin(r), std::ranges::end(r), out, expr);
    }
};

inline constexpr parse_words_fn parseWords;

inline auto startsWith(const std::string& str, const std::string& start) -> bool
{
    if (start.size() > str.size())
        return false;
    return std::equal(start.cbegin(), start.cend(), str.cbegin());
}

inline auto endsWith(const std::string& str, const std::string& end) -> bool
{
    if (end.size() > str.size())
        return false;
    return std::equal(end.crbegin(), end.crend(), str.crbegin());
}

inline auto formatDecimal(double value, int precision) -> std::string
{
    std::stringstream ss;
    ss << std::fixed << std::setprecision(precision) << value;
    return ss.str();
}

inline auto split(std::string_view str, char delimeter)
    -> std::vector<std::string_view>
{
    std::vector<std::string_view> result;
    for (size_t left{0}, right{0}; right != std::string_view::npos;
         left = right + 1) {
        right = str.find_first_of(delimeter, left);
        result.push_back(str.substr(left, right - left));
    }
    return result;
}

} // namespace alg

#endif /* end of include guard: STRING_EXT_H_0CNTGIMQ */

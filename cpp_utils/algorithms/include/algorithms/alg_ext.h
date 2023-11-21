#ifndef ALG_EXT_H_NVSY3TXB
#define ALG_EXT_H_NVSY3TXB

#include <algorithm>
#include <functional>

namespace alg {

/* Slide elements inside of container.
 * Return pair of iterators to first and last elements of moved sequence.
 */
template <typename T> auto slide(T first, T last, T position) -> std::pair<T, T>
{
    if (position < last)
        return {position, std::rotate(position, first, last)};
    if (last < position)
        return {std::rotate(first, last, position), position};
    return {first, last};
}

/* Output all pairs of adjacent elements that satisfy BinaryPredicate.
 * Returns iterator to the element past the last element written. */
struct find_all_adjacent_matches_fn {

    template <std::forward_iterator I,
              std::sentinel_for<I> S,
              std::weakly_incrementable Out,
              class Proj = std::identity,
              std::indirect_binary_predicate<std::projected<I, Proj>,
                                             std::projected<I, Proj>> Pred =
                  std::ranges::equal_to>
    constexpr auto
    operator()(I first, S last, Out out, Pred pred = {}, Proj proj = {}) const
        -> I
    {
        for (first = std::ranges::adjacent_find(first, last, pred, proj);
             first != last;
             first = std::ranges::adjacent_find(
                 std::next(first), last, pred, proj)) {
            *(++out) = {*first, *std::next(first)};
        }
        return first;
    }

    template <std::ranges::forward_range R,
              std::weakly_incrementable Out,
              class Proj = std::identity,
              std::indirect_binary_predicate<
                  std::projected<std::ranges::iterator_t<R>, Proj>,
                  std::projected<std::ranges::iterator_t<R>, Proj>> Pred =
                  std::ranges::equal_to>
    constexpr auto
    operator()(R&& r, Out out, Pred pred = {}, Proj proj = {}) const
        -> std::ranges::borrowed_iterator_t<R>
    {
        return (*this)(std::ranges::begin(r),
                       std::ranges::end(r),
                       out,
                       std::ref(pred),
                       std::ref(proj));
    }
};

inline constexpr find_all_adjacent_matches_fn find_all_adjacent_matches;

} // namespace alg

#endif /* end of include guard: ALG_EXT_H_NVSY3TXB */

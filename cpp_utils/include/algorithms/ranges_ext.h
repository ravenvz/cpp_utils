#ifndef RANGES_EXT_H_1MXEJNIG
#define RANGES_EXT_H_1MXEJNIG

#include <concepts>
#include <iterator>
#include <ranges>

namespace alg {

template <class F, class T, class U>
concept foldable = std::regular_invocable<F&, T, U> &&
    std::convertible_to<std::invoke_result_t<F&, T, U>, T>;

template <class F, class T, class I>
concept indirectly_binary_foldable = std::indirectly_readable<I> &&
    std::copy_constructible<F> && foldable<F, T, std::iter_value_t<I>> &&
    foldable<F, T, std::iter_reference_t<I>> &&
    foldable<F, T, std::iter_common_reference_t<I>>;

struct fold_fn {

    template <
        std::input_iterator I,
        std::sentinel_for<I> S,
        std::movable T,
        class Proj = std::identity,
        indirectly_binary_foldable<T, std::projected<I, Proj>> BinaryOperation>
    constexpr T operator()(
        I first, S last, T init, BinaryOperation op, Proj proj = {}) const
    {
        for (; first != last; ++first) {
            init = op(std::move(init), proj(*first));
        }
        return init;
    }

    template <
        std::ranges::input_range R,
        std::movable T,
        class Proj = std::identity,
        indirectly_binary_foldable<
            T,
            std::projected<std::ranges::iterator_t<R>, Proj>> BinaryOperation>
    constexpr T
    operator()(R&& r, T init, BinaryOperation op, Proj proj = {}) const
    {
        return (*this)(std::ranges::begin(r),
                       std::ranges::end(r),
                       std::move(init),
                       std::ref(op),
                       std::ref(proj));
    }
};

inline constexpr fold_fn fold;

} // namespace alg

#endif /* end of include guard: RANGES_EXT_H_1MXEJNIG */

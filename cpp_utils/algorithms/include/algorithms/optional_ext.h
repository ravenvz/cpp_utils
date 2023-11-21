#ifndef OPTIONAL_EXT_H_OPKE4LGJ
#define OPTIONAL_EXT_H_OPKE4LGJ

#include <functional>
#include <optional>

namespace alg {

template <typename Opt, typename Func>
constexpr auto inspect(Opt&& opt, Func&& func) -> void
{
    if (opt) {
        func(*std::forward<Opt>(opt));
    }
}

template <typename T, typename Comp>
constexpr auto opt_equal(const std::optional<T>& lhs, const std::optional<T>& rhs, Comp&& comp) {
    return (!lhs && !rhs) || (lhs && rhs && comp(*lhs, *rhs));
}

} // namespace alg

#endif /* end of include guard: OPTIONAL_EXT_H_OPKE4LGJ */

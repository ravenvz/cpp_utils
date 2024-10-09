#ifndef NAMEDTYPE_H_SMAVR32Q
#define NAMEDTYPE_H_SMAVR32Q

#include <utility>

namespace types {

template <typename T, typename Tag> class ImplicitNamedType {
public:
    constexpr ImplicitNamedType() = default;

    explicit constexpr ImplicitNamedType(T&& val_) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : val{std::move(val_)}
    {
    }

    explicit constexpr ImplicitNamedType(const T& val_)
        : val{val_}
    {
    }

    constexpr operator const T&() const noexcept { return val; }

    constexpr operator T&() noexcept { return val; }

private:
    T val;
};

template <typename T, typename Tag> class ExplicitNamedType {
public:
    constexpr ExplicitNamedType() = default;

    explicit constexpr ExplicitNamedType(T&& val_) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : val{std::move(val_)}
    {
    }

    explicit constexpr ExplicitNamedType(const T& val_)
        : val{val_}
    {
    }

    explicit constexpr operator const T&() const noexcept { return val; }

    explicit constexpr operator T&() noexcept { return val; }

private:
    T val;
};

} // namespace types

#endif /* end of include guard: NAMEDTYPE_H_SMAVR32Q */


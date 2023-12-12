#ifndef NAMEDTYPE_H_SMAVR32Q
#define NAMEDTYPE_H_SMAVR32Q

#include <utility>

namespace types {

template <typename T, typename Tag> class ImplicitNamedType {
public:
    ImplicitNamedType() = default;

    explicit ImplicitNamedType(T&& val_) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : val{std::move(val_)}
    {
    }

    explicit ImplicitNamedType(const T& val_)
        : val{val_}
    {
    }

    operator const T&() const noexcept { return val; }

    operator T&() noexcept { return val; }

private:
    T val;
};

template <typename T, typename Tag> class ExplicitNamedType {
public:
    ExplicitNamedType() = default;

    explicit ExplicitNamedType(T&& val_) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : val{std::move(val_)}
    {
    }

    explicit ExplicitNamedType(const T& val_)
        : val{val_}
    {
    }

    explicit operator const T&() const noexcept { return val; }

    explicit operator T&() noexcept { return val; }

private:
    T val;
};

} // namespace types

#endif /* end of include guard: NAMEDTYPE_H_SMAVR32Q */


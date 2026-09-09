#ifndef NAMEDTYPE_H_SMAVR32Q
#define NAMEDTYPE_H_SMAVR32Q

#include <concepts>
#include <format>
#include <type_traits>
#include <utility>

/**
 * StrongType: A header-only, zero-overhead, type-safe wrapper framework.
 *
 * ============================================================================
 *                                INTRODUCTION
 * ============================================================================
 * StrongType is a zero-overhead type-safe wrapper framework designed to prevent
 * semantic bugs in C++ applications. It allows you to create distinct types for
 * identical underlying primitives (e.g., separating an Apple count from a
 * UserId or a Velocity metric), ensuring they cannot be accidentally mixed,
 * compared, or assigned to one another.
 *
 * ============================================================================
 *                          CORE DESIGN PHILOSOPHY
 * ============================================================================
 * 1. Zero Runtime Overhead:
 *    Leveraging the Empty Base Optimization (EBO) and modern C++ compiler
 * features, wrapping a primitive does not alter its memory footprint.
 *    sizeof(StrongType<int, Tag>) is identical to sizeof(int).
 *
 * 2. Opt-In Capabilities (Mixins):
 *    By default, a StrongType is a strict, read-only container. It behaves
 *    mathematically only if you explicitly compose it with specialized
 * "Extensions" or bundled capabilities.
 *
 * 3. Strict Homogeneous Operations:
 *    All operations are strictly guarded. An Apple cannot interact with a raw
 *    primitive int or a different strong type, forcing clear, explicit code
 *    correctness boundaries.
 *
 * ============================================================================
 *                               BASIC USAGE
 * ============================================================================
 * To instantiate a strong type, define an incomplete struct tag to isolate your
 * domain name, and specify the baseline extension mixins (if required).
 *
 * --- 1. Basic Structural Identifier ---
 * By default, types automatically inherit basic equivalence checking (==) and
 * spaceship sorting order (<=>). If no extra skills are provided, the type is
 * immutable and completely isolated from outer mathematical arithmetic.
 *
 *   struct UserIdTag {};
 *   struct TransactionIdTag {};
 *
 *   using UserId = StrongType<int, UserIdTag>;
 *   using TransactionId = StrongType<int, TransactionIdTag>;
 *
 *   void process_user(UserId id);
 *
 *   int main() {
 *       UserId user{42};
 *       TransactionId tx{42};
 *
 *       // process_user(tx);   // COMPILATION ERROR! Type mismatch protected.
 *       // if (user == tx) {}  // COMPILATION ERROR! Cannot mix contexts.
 *
 *       if (user == UserId{42}) {  Valid homogeneous evaluation  }
 *   }
 *
 * --- 2. Full Numeric Behaviors ---
 * If your strong type models a numerical quantity (like an accumulation,
 * weight, or distance metric), attach the NumericRepresentation master bundle.
 * This unlocks homogeneous math (+, -, *, /), increments (++, --), and compound
 * modifications (+=, -=, etc.).
 *
 *   struct AppleTag {};
 *   using Apple = StrongType<int, AppleTag, NumericRepresentation>;
 *
 *   int main() {
 *       Apple basket{10};
 *       Apple hand{5};
 *
 *       Apple total = basket + hand; // Valid: Apple{15}
 *       total += Apple{2};           // Valid: Apple{17}
 *       ++total;                     // Valid: Apple{18}
 *
 *       // Apple bad = total + 5;    // COMPILATION ERROR! No implicit
 * primitive leaks. Apple safe = total + Apple{5}; // Valid: Self-documenting
 * domain operation.
 *   }
 *
 * --- 3. Restricting Features (Opt-Out) ---
 * For sensitive contexts where tracking order or equivalence checking poses an
 * architectural violation (e.g., cryptographic contexts, non-copyable
 * pointers), you can pass the NonComparable mixin tag to shut down default
 * spaceship operators cleanly.
 *
 *   struct SecretBufferTag {};
 *   using SafeBuffer = StrongType<std::vector<char>, SecretBufferTag,
 * NonComparable>;
 *
 *   int main() {
 *       SafeBuffer a{ {'X'} };
 *       SafeBuffer b{ {'Y'} };
 *
 *       // if (a == b) {} // COMPILATION ERROR! Comparison capability has been
 * stripped.
 *   }
 *
 * ============================================================================
 *                            EXTENDING THE LIBRARY
 * ============================================================================
 * The design follows a standard CRTP layout. If you need a custom capability
 * tailored to your repository pipeline, you can easily author a custom mixin
 * class following this exact blueprint:
 *
 *   template <typename Derived>
 *   struct CustomSkill {
 *       // Access wrapped immutable state using:
 *       //   static_cast<const Derived&>(*this).get()
 *       // Access wrapped mutable state using:
 *       //   static_cast<Derived&>(*this).numeric_bundle_get_mut()
 *   };
 */

namespace cpp_utils::strong_type {

// Validates if the underlying type supports standard arithmetic operations.
template <typename T>
concept IsNumeric = std::integral<T> || std::floating_point<T>;

// Mixin tag to explicitly disable relational comparison operators on a
// StrongType.
template <typename Derived> struct NonComparable { };

// Mixin tag to enable automatic std::hash specialization.
template <typename Derived> struct Hashable { };

// Injects homogeneous addition behavior into the StrongType.
template <typename Derived> struct Addable {

    friend constexpr auto operator+(const Derived& lhs, const Derived& rhs)
        -> Derived
    {
        return Derived{lhs.get() + rhs.get()};
    }
};

// Injects strictly homogeneous subtraction behavior into the StrongType.
template <typename Derived> struct Subtractable {

    friend constexpr auto operator-(const Derived& lhs, const Derived& rhs)
        -> Derived
    {
        return Derived{lhs.get() - rhs.get()};
    }
};

// Injects homogeneous multiplication and division behavior
// into the StrongType.
template <typename Derived> struct Multiplicable {

    friend constexpr auto operator*(const Derived& lhs, const Derived& rhs)
        -> Derived
    {
        return Derived{lhs.get() * rhs.get()};
    }

    friend constexpr auto operator/(const Derived& lhs, const Derived& rhs)
        -> Derived
    {
        return Derived{lhs.get() / rhs.get()};
    }
};

// Injects strictly homogeneous compound arithmetic assignment behavior
// into the StrongType.
template <typename Derived> struct Scalable {

    constexpr auto operator+=(const Derived& rhs) -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        // Triggers unshadowed bundle lookup
        self.numeric_bundle_get_mut() += rhs.get();
        return self;
    }

    constexpr auto operator-=(const Derived& rhs) -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        self.numeric_bundle_get_mut() -= rhs.get();
        return self;
    }

    constexpr auto operator*=(const Derived& rhs) -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        self.numeric_bundle_get_mut() *= rhs.get();
        return self;
    }

    constexpr auto operator/=(const Derived& rhs) -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        self.numeric_bundle_get_mut() /= rhs.get();
        return self;
    }
};

// Injects pre- and post-increment capabilities into the StrongType.
template <typename Derived> struct Incrementable {

    constexpr auto operator++() -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        ++self.numeric_bundle_get_mut();
        return self;
    }

    constexpr auto operator++(int) -> Derived
    {
        auto& self = static_cast<Derived&>(*this);
        Derived tmp = self;
        ++(*this);
        return tmp;
    }
};

// Injects pre- and post-decrement capabilities into the StrongType.
template <typename Derived> struct Decrementable {

    constexpr auto operator--() -> Derived&
    {
        auto& self = static_cast<Derived&>(*this);
        --self.numeric_bundle_get_mut();
        return self;
    }

    constexpr auto operator--(int) -> Derived
    {
        auto& self = static_cast<Derived&>(*this);
        Derived tmp = self;
        --(*this);
        return tmp;
    }
};

// Aggregates full numeric mutations, modifications, and algebra
// operations.
template <typename Derived>
struct NumericRepresentation : Addable<Derived>,
                               Subtractable<Derived>,
                               Multiplicable<Derived>,
                               Scalable<Derived>,
                               Incrementable<Derived>,
                               Decrementable<Derived> {
    // Public unique accessor for sub-mixins to reach get_mut() without
    // shadowing.
    constexpr auto numeric_bundle_get_mut() -> decltype(auto)
    {
        // Allowed because NumericRepresentation is a direct friend
        return static_cast<Derived&>(*this).get_mut();
    }
};

// --- Inside StrongType.hpp ---
#if defined(_MSC_VER)
#define CPP_UTILS_MSVC_EBO __declspec(empty_bases)
#else
#define CPP_UTILS_MSVC_EBO
#endif

// Encapsulates an underlying primitive or object into a type-safe
// wrapper.
template <std::default_initializable T,
          typename Tag,
          template <typename> class... Extensions>
class CPP_UTILS_MSVC_EBO StrongType
    : public Extensions<StrongType<T, Tag, Extensions...>>... {

    static constexpr bool is_non_comparable =
        (std::is_same_v<Extensions<StrongType>, NonComparable<StrongType>> ||
         ...);

public:
    constexpr StrongType() = default;

    explicit constexpr StrongType(T&& val_) noexcept(
        std::is_nothrow_move_constructible_v<T>)
        : val{std::move(val_)}
    {
        if constexpr ((std::is_base_of_v<NumericRepresentation<StrongType>,
                                         Extensions<StrongType>> ||
                       ...)) {
            static_assert(IsNumeric<T>,
                          "NumericRepresentation bundle can only be applied to "
                          "numeric types!");
        }
    }

    explicit constexpr StrongType(const T& val_)
        : val{val_}
    {
        if constexpr ((std::is_base_of_v<NumericRepresentation<StrongType>,
                                         Extensions<StrongType>> ||
                       ...)) {
            static_assert(IsNumeric<T>,
                          "NumericRepresentation bundle can only be applied to "
                          "numeric types!");
        }
    }

    // Read-only accessor to retrieve the encapsulated value.
    constexpr auto get() const -> const T& { return val; }

    // Explicit conversion operator to cast down to the underlying type.
    constexpr explicit operator const T&() const noexcept { return val; }

    // ==========================================
    // Relation and Equivalence Operators
    // ==========================================

    friend constexpr auto operator<=>(const StrongType& lhs,
                                      const StrongType& rhs)
        requires(!is_non_comparable)
    {
        return lhs.val <=> rhs.val;
    }

    friend constexpr auto operator==(const StrongType& lhs,
                                     const StrongType& rhs) -> bool
        requires(!is_non_comparable)
    {
        return lhs.val == rhs.val;
    }

    // Fallback named bridge for standalone granular mixins (like
    // Incrementable used directly).
    constexpr auto numeric_bundle_get_mut() -> T& { return val; }

private:
    T val{};

    // Private mutable accessor reserved for verified modifying skill
    // sets.
    constexpr auto get_mut() -> T& { return val; }
};

} // namespace cpp_utils::strong_type

template <typename T, typename Tag, template <typename> class... Extensions>
struct std::formatter<cpp_utils::strong_type::StrongType<T, Tag, Extensions...>>
    : std::formatter<T> {
    auto
    format(const cpp_utils::strong_type::StrongType<T, Tag, Extensions...>& obj,
           format_context& ctx) const
    {
        return std::formatter<T>::format(obj.get(), ctx);
    }
};

template <typename T, typename Tag, template <typename> class... Extensions>
struct std::hash<cpp_utils::strong_type::StrongType<T, Tag, Extensions...>> {
    auto operator()(
        const cpp_utils::strong_type::StrongType<T, Tag, Extensions...>& obj)
        const noexcept -> std::size_t
    {
        return std::hash<T>{}(obj.get());
    }
};

#endif

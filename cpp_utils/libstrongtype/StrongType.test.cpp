#include "gmock/gmock.h"
#include <cpp_utils/libstrongtype/StrongType.hpp>
#include <unordered_map>
#include <vector>

using namespace cpp_utils::strong_type;

using Apple = StrongType<int, struct AppleTag, NumericRepresentation>;
using Distance = StrongType<double, struct DistanceTag, NumericRepresentation>;
using UserId = StrongType<int, struct IdTag>; // Default features (Comparable)
using SafeBuffer =
    StrongType<std::vector<char>, struct BufferTag, NonComparable>;

// Helper macros to make static_assert testing highly readable inside gtest
// source
#define STATIC_ASSERT_TRUE(Concept)                                            \
    static_assert(Concept, "Concept validation failed: should be true")
#define STATIC_ASSERT_FALSE(Concept)                                           \
    static_assert(!Concept, "Concept validation failed: should be false")

TEST(StrongTypeCoreTest, BasicProperties)
{
    Apple a{10};
    EXPECT_EQ(a.get(), 10);

    // Default construction check
    Apple default_a;
    EXPECT_EQ(default_a.get(), 0);

    // Size check to verify Zero Overhead / Empty Base Optimization (EBO)
    EXPECT_EQ(sizeof(Apple), sizeof(int));
    EXPECT_EQ(sizeof(UserId), sizeof(int));
}

TEST(StrongTypeCoreTest, TypeSafetyCompileChecks)
{
    // Verify that distinct tags create distinct, non-convertible types
    bool is_same_type = std::is_same_v<Apple, UserId>;
    EXPECT_FALSE(is_same_type);

    // Verify explicit conversion operators
    Apple a{5};
    int raw_val = static_cast<int>(a);
    EXPECT_EQ(raw_val, 5);
}

TEST(StrongTypeComparisonTest, DefaultSpaceshipOperators)
{
    UserId id1{42};
    UserId id2{100};
    UserId id3{42};

    // Homogeneous comparisons should work by default
    EXPECT_TRUE(id1 < id2);
    EXPECT_TRUE(id2 > id1);
    EXPECT_TRUE(id1 == id3);
    EXPECT_TRUE(id1 != id2);
    EXPECT_TRUE(id1 <= id2);
    EXPECT_TRUE(id2 >= id1);
}

// Helper traits to test concept rejection of comparisons
template <typename T>
concept CanBeEqualCompared = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
};

template <typename T>
concept CanBeLessCompared = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
};

TEST(StrongTypeComparisonTest, NonComparableOptOut)
{
    // Assert at compile-time using concepts that SafeBuffer rejected operators
    STATIC_ASSERT_FALSE(CanBeEqualCompared<SafeBuffer>);
    STATIC_ASSERT_FALSE(CanBeLessCompared<SafeBuffer>);

    // Concurrently verify that UserId/Apple still maintain comparisons
    STATIC_ASSERT_TRUE(CanBeEqualCompared<UserId>);
    STATIC_ASSERT_TRUE(CanBeLessCompared<Apple>);
}

TEST(StrongTypeArithmeticTest, BasicHomogeneousMath)
{
    Apple a{20};
    Apple b{10};

    Apple sum = a + b;
    EXPECT_EQ(sum.get(), 30);

    Apple diff = a - b;
    EXPECT_EQ(diff.get(), 10);

    Apple prod = a * b;
    EXPECT_EQ(prod.get(), 200);

    Apple quot = a / b;
    EXPECT_EQ(quot.get(), 2);
}

TEST(StrongTypeArithmeticTest, CompoundAssignment)
{
    Apple a{10};

    a += Apple{5};
    EXPECT_EQ(a.get(), 15);

    a -= Apple{3};
    EXPECT_EQ(a.get(), 12);

    a *= Apple{2};
    EXPECT_EQ(a.get(), 24);

    a /= Apple{4};
    EXPECT_EQ(a.get(), 6);
}

TEST(StrongTypeSteppingTest, IncrementDecrement)
{
    Apple a{10};

    // Pre-increment
    Apple& ref = ++a;
    EXPECT_EQ(a.get(), 11);
    EXPECT_EQ(ref.get(), 11);

    // Post-increment
    Apple old_post = a++;
    EXPECT_EQ(old_post.get(), 11);
    EXPECT_EQ(a.get(), 12);

    // Pre-decrement
    --a;
    EXPECT_EQ(a.get(), 11);

    // Post-decrement
    Apple old_dec = a--;
    EXPECT_EQ(old_dec.get(), 11);
    EXPECT_EQ(a.get(), 10);
}

TEST(StrongTypeFormattingTest, StandardFormatIntegration)
{
    Apple a{5};
    std::string formatted_basic = std::format("{}", a);
    EXPECT_EQ(formatted_basic, "5");

    // Padding & Specifier tests inherited automatically from inner type 'int'
    std::string formatted_padded = std::format("{:03d}", a);
    EXPECT_EQ(formatted_padded, "005");

    // Float specifier forwarding via Distance
    Distance d{12.3456};
    std::string formatted_float = std::format("{:.2f}", d);
    EXPECT_EQ(formatted_float, "12.35");
}

TEST(StrongTypeFormattingTest, HashableExtension)
{
    using Pear = StrongType<int, struct PearTag, Hashable>;
    std::unordered_map<Pear, int> mp;

    mp.emplace(Pear{2}, 77);

    EXPECT_EQ(77, mp[Pear{2}]);
}

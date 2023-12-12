#include "cpp_utils/algorithms/string_ext.h"
#include <gmock/gmock.h>

using namespace alg;

TEST(TestStringUtils, test_parse_words)
{
    const std::string text{
        "Some    text1, ?that! should?Be,parsed... in C++ ..D--  "};
    const std::vector<std::string> expected{
        "Some", "text1", "that", "should", "Be", "parsed", "in", "C++", "D--"};

    std::vector<std::string> actual;
    parseWords(cbegin(text), cend(text), std::back_inserter(actual));

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_join_empty_container)
{
    std::vector<std::string> parts;
    const std::string expected{""};

    const std::string actual = join(parts, ", ");

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_join_with_some_parts_empty)
{
    const std::vector<std::string> parts{"Some", "", "are", "empty"};
    const std::string expected{"Some are empty"};

    std::string actual = join(parts, " ");

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_join_with_all_parts_empty)
{
    const std::vector<std::string> parts{"", "", "", ""};
    const std::string expected{""};

    const std::string actual = join(parts, " ");

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_join_on_list_of_ints)
{
    const std::list<int> parts{1, 2, 3, 4};
    const std::string expected{"1 -> 2 -> 3 -> 4"};

    const std::string actual = join(parts.cbegin(), parts.cend(), " -> ");

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_transform_join_on_strings)
{
    const std::vector<std::string> parts{"Some", "", "are", "empty"};
    const std::string expected{"#Some #are #empty"};

    const std::string actual = join(parts, " ", [](const auto& elem) {
        if (elem.empty()) {
            return std::string{};
        }
        std::string res{"#"};
        res += elem;
        return res;
    });

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_transform_join_on_ints)
{
    const std::vector<int> parts{1, 2, 3, 4};
    const std::string expected{"1 + 4 + 9 + 16"};

    const std::string actual =
        join(parts, " + ", [](int num) { return num * num; });

    EXPECT_EQ(expected, actual);
}

TEST(TestStringUtils, test_starts_with)
{
    EXPECT_TRUE(startsWith("whatever", "what"));
    EXPECT_TRUE(startsWith("what", ""));
    EXPECT_TRUE(startsWith("", ""));

    EXPECT_FALSE(startsWith("what", "whatever"));
    EXPECT_FALSE(startsWith("", "what"));
}

TEST(TestStringUtils, test_ends_with)
{
    EXPECT_TRUE(endsWith("whatever", "ever"));
    EXPECT_TRUE(endsWith("whatever", ""));
    EXPECT_TRUE(endsWith("", ""));

    EXPECT_FALSE(endsWith("whatever", "everest"));
    EXPECT_FALSE(endsWith("", "ever"));
    EXPECT_FALSE(endsWith("ever", "whatever"));
}

TEST(TestStringUtils, test_split)
{
    EXPECT_THAT(split("I,am,csv,data", ','),
                ::testing::ElementsAre("I", "am", "csv", "data"));
    EXPECT_THAT(split("Singleword", ','), ::testing::ElementsAre("Singleword"));
    EXPECT_THAT(split(",", ','),
                ::testing::ElementsAre(std::string{}, std::string{}));
}

TEST(TestStringUtils, test_split_overload)
{
    EXPECT_THAT(split("I..am..strange..data", ".."),
                ::testing::ElementsAre("I", "am", "strange", "data"));
}

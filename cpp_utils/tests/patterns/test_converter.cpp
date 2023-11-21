#include "algorithms/string_ext.h"
#include "patterns/Converter.h"
#include "gtest/gtest.h"

using namespace patterns;

namespace {

class SomeType {
public:
    std::string name;
    int value{0};

    friend bool operator==(const SomeType&, const SomeType&) = default;
};

class SomeTypeDTO {
public:
    std::string some_parameter;

    friend bool operator==(const SomeTypeDTO&, const SomeTypeDTO&) = default;
};

class SomeTypeConverter : public Converter<SomeTypeDTO, SomeType> {
private:
    auto make_dto_impl(SomeType const& entity) const -> SomeTypeDTO override
    {
        return SomeTypeDTO{entity.name + std::string{" "} +
                           std::to_string(entity.value)};
    }

    auto make_entity_impl(SomeTypeDTO const& dto) const -> SomeType override
    {
        const auto parts = alg::split(dto.some_parameter, ' ');
        return SomeType{std::string{parts[0]},
                        std::stoi(std::string{parts[1]})};
    }
};

} // namespace

class ConverterFixture : public ::testing::Test {
public:
};

TEST_F(ConverterFixture, test_name)
{
    SomeType entity{"Hello", 3};
    SomeTypeDTO dto{"Hello 3"};

    SomeTypeConverter converter;

    EXPECT_EQ(dto, converter(entity));
    EXPECT_EQ(entity, converter(dto));
}

TEST_F(ConverterFixture, convert_container)
{
    const std::vector<SomeType> entities{
        SomeType{"First", 1}, SomeType{"Second", 2}, SomeType{"Third", 3}};
    const std::vector<SomeTypeDTO> dtos{SomeTypeDTO{"First 1"},
                                        SomeTypeDTO{"Second 2"},
                                        SomeTypeDTO{"Third 3"}};

    SomeTypeConverter converter;

    EXPECT_TRUE(std::ranges::equal(dtos, converter(entities)));
    EXPECT_TRUE(std::ranges::equal(entities, converter(dtos)));
}

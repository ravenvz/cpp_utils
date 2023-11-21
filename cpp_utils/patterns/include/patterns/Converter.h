#ifndef CONVERTER_H_CKHQKY9G
#define CONVERTER_H_CKHQKY9G

#include <ranges>
#include <span>

namespace patterns {

template <typename Dto, typename Entity> class Converter {
public:
    virtual ~Converter() = default;

    auto operator()(const Entity& entity) const -> Dto
    {
        return make_dto_impl(entity);
    }

    auto operator()(const Dto& dto) const -> Entity
    {
        return make_entity_impl(dto);
    }

    auto operator()(std::span<const Entity> entities) const
    {
        return std::views::transform(entities, [this](const auto& entity) {
            return operator()(entity);
        });
    }

    auto operator()(std::span<const Dto> dtos) const
    {
        return std::views::transform(
            dtos, [this](const auto& dto) { return operator()(dto); });
    }

private:
    virtual auto make_dto_impl(Entity const&) const -> Dto = 0;

    virtual auto make_entity_impl(Dto const&) const -> Entity = 0;
};

} // namespace patterns

#endif /* end of include guard: CONVERTER_H_CKHQKY9G */

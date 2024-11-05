#ifndef STATE_H_S8VQS7DT
#define STATE_H_S8VQS7DT

#include <variant>

namespace details {

template <typename Transitions> struct ExternalTransitions {
    explicit ExternalTransitions(Transitions transitions_)
        : transitions(std::move(transitions_))
    {
    }
    template <typename State, typename Event>
    auto execute(State& state, Event&& event)
    {
        return transitions.operator()(state, std::forward<Event>(event));
    }

private:
    Transitions transitions{};
};

struct StatesHandlingTransitions {
    template <typename State, typename Event>
    auto execute(State& state, Event&& event)
    {
        return state.process(std::forward<Event>(event));
    }
};

template <typename Strategy, typename... States> class FSMBase {
public:
    template <typename InitialState>
    explicit FSMBase(Strategy strategy_, InitialState&& state_)
        : strategy{std::move(strategy_)}
        , state{std::forward<InitialState>(state_)}
    {
    }

    template <typename Event> auto process(Event event) -> void
    {
        auto res = std::visit(
            [&](auto& state_) {
                return strategy.execute(state_, std::move(event));
            },
            state);
        if (res) {
            state = std::move(*res);
        }
    }

    auto get_state() const
    {
        return std::visit([](auto& state_) { return state_.get_state(); },
                          state);
    }

protected:
    Strategy strategy;
    std::variant<States...> state;
};

} // namespace details

namespace patterns {

template <typename... States>
class FSMStateTransitions
    : public details::FSMBase<details::StatesHandlingTransitions, States...> {
    using BaseType =
        details::FSMBase<details::StatesHandlingTransitions, States...>;
    using StrategyType = details::StatesHandlingTransitions;

public:
    template <typename InitialState>
    explicit FSMStateTransitions(InitialState&& state)
        : BaseType{StrategyType{}, std::forward<InitialState>(state)}
    {
    }
};

template <typename Transitions, typename... States>
class FSMExternalTransitions
    : public details::FSMBase<details::ExternalTransitions<Transitions>,
                              States...> {
    using BaseType =
        details::FSMBase<details::ExternalTransitions<Transitions>, States...>;
    using StrategyType = details::ExternalTransitions<Transitions>;

public:
    template <typename InitialState>
    explicit FSMExternalTransitions(Transitions transitions,
                                    InitialState&& state)
        : BaseType{StrategyType{std::move(transitions)},
                   std::forward<InitialState>(state)}
    {
    }
};

} // namespace patterns

#endif /* end of include guard: STATE_H_S8VQS7DT */

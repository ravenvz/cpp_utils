#include "gmock/gmock.h"

#include "common_state.h"
#include "cpp_utils/patterns/State.h"
#include <optional>
#include <string_view>
#include <variant>

namespace external_state {

template <typename Context> class AutoOpened;

template <typename Context> class AutoClosed;

template <typename Context> class Opened;

template <typename Context> class Closed;

template <typename Context> class BaseState {
public:
    explicit BaseState(std::reference_wrapper<Context> context_)
        : context{context_}
    {
    }

protected:
    std::reference_wrapper<Context> context;
};

template <typename Context> class AutoOpened : public BaseState<Context> {
public:
    using BaseState<Context>::context;

    AutoOpened(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().open();
        context.get().get_gate().unlock();
        context.get().set_status("automatic");
    }

    auto get_state() const -> States { return States::AutoOpened; }
};

template <typename Context> class AutoClosed : public BaseState<Context> {
public:
    using BaseState<Context>::context;

    AutoClosed(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().close();
        context.get().get_gate().unlock();
        context.get().set_status(std::string{"automatic"});
    }

    auto get_state() const -> States { return States::AutoClosed; }

    auto active_range(int distance) -> bool
    {
        return distance <= context.get().get_threshold();
    }
};

template <typename Context> class Opened : public BaseState<Context> {
public:
    using BaseState<Context>::context;

    Opened(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().open();
        context.get().get_gate().lock();
        context.get().set_status("manual");
    }

    auto get_state() const -> States { return States::Opened; }
};

template <typename Context> class Closed : public BaseState<Context> {
public:
    using BaseState<Context>::context;

    Closed(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().close();
        context.get().get_gate().lock();
        context.get().set_status("manual");
    }

    auto get_state() const -> States { return States::Closed; }
};

} // namespace external_state

namespace external_transitions {

class GateController;

using AutoOpened = external_state::AutoOpened<GateController>;
using AutoClosed = external_state::AutoClosed<GateController>;
using Opened = external_state::Opened<GateController>;
using Closed = external_state::Closed<GateController>;

using State = std::variant<AutoOpened, AutoClosed, Opened, Closed>;

using MaybeState = std::optional<State>;

struct TransitionTable {
    auto operator()(AutoOpened& state, PersonLeftRange) -> MaybeState
    {
        return AutoClosed{state.context};
    }

    auto operator()(AutoOpened& state, ManualCloseSet) -> MaybeState
    {
        return Closed{state.context};
    }

    auto operator()(AutoOpened& state, ManualOpenSet) -> MaybeState
    {
        return Opened{state.context};
    }

    auto operator()(AutoClosed& state, PersonInRange event) -> MaybeState
    {
        return state.active_range(event.distance) ? AutoOpened{state.context}
                                                  : MaybeState{};
    }

    auto operator()(AutoClosed& state, ManualCloseSet) -> MaybeState
    {
        return Closed{state.context};
    }

    auto operator()(AutoClosed& state, ManualOpenSet) -> MaybeState
    {
        return Opened{state.context};
    }

    auto operator()(Opened& state, AutoModeEngaged) -> MaybeState
    {
        return AutoOpened{state.context};
    }

    auto operator()(Opened& state, ManualCloseSet) -> MaybeState
    {
        return Closed{state.context};
    }

    auto operator()(Closed& state, ManualOpenSet) -> MaybeState
    {
        return Opened{state.context};
    }

    auto operator()(Closed& state, AutoModeEngaged) -> MaybeState
    {
        return AutoClosed{state.context};
    }

    template <typename StateT, typename EventT>
    auto operator()(StateT&, EventT) const -> MaybeState
    {
        return MaybeState{};
    }
};

class GateController {
public:
    explicit GateController(int threshold_distance_)
        : threshold_distance{threshold_distance_}
        , fsm{TransitionTable{}, AutoClosed{std::ref(*this)}}
    {
    }

    template <typename EventT> auto process(EventT event) -> GateController&
    {
        fsm.process(std::move(event));
        return *this;
    }

    auto get_gate() -> Gate& { return gate; }

    auto get_status() const -> std::string_view { return status; }

    auto get_threshold() const -> int { return threshold_distance; }

    auto set_status(std::string updated_status) -> void
    {
        status = std::move(updated_status);
    }

    auto get_state() const -> States { return fsm.get_state(); }

private:
    int threshold_distance;
    Gate gate;
    std::string status{"automatic"};
    patterns::FSMExternalTransitions<TransitionTable,
                                     AutoOpened,
                                     AutoClosed,
                                     Opened,
                                     Closed>
        fsm;
};

} // namespace external_transitions

class ExternalTransitionFixture : public ::testing::Test {
public:
    external_transitions::GateController fsm{3};
};

TEST_F(ExternalTransitionFixture, initial_state)
{
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(ExternalTransitionFixture,
       autoopen_gate_should_not_react_when_range_greater_than_treshold)
{
    fsm.process(PersonInRange{4});

    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, autoclosed_transitions_to_autoopen)
{
    fsm.process(PersonInRange{3});

    EXPECT_EQ(States::AutoOpened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, autoopen_transitions_to_autoclose)
{
    fsm.process(PersonInRange{3});

    fsm.process(PersonLeftRange{});

    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, autoclosed_transitions_to_closed)
{
    fsm.process(ManualCloseSet{});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, closed_state_ignores_detection_event)
{
    fsm.process(ManualCloseSet{});

    fsm.process(PersonInRange{3});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, autoclosed_transitions_to_opened)
{
    fsm.process(ManualOpenSet{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, autoopen_transitions_to_opened)
{
    fsm.process(PersonInRange{3});

    fsm.process(ManualOpenSet{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, opened_ignores_person_left_range_event)
{
    fsm.process(ManualOpenSet{});

    fsm.process(PersonLeftRange{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, opened_transitions_to_closed)
{
    fsm.process(ManualOpenSet{});

    fsm.process(ManualCloseSet{});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, closed_transitions_to_opened)
{
    fsm.process(ManualCloseSet{});

    fsm.process(ManualOpenSet{});
    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, opened_transitions_to_autoopened)
{
    fsm.process(ManualOpenSet{});

    fsm.process(AutoModeEngaged{});

    EXPECT_EQ(States::AutoOpened, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(ExternalTransitionFixture, closed_transitions_to_autoclosed)
{
    fsm.process(ManualCloseSet{});

    fsm.process(AutoModeEngaged{});
    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

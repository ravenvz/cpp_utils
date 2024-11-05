#include "gmock/gmock.h"

#include "common_state.h"
#include "cpp_utils/patterns/State.h"
#include <optional>
#include <string_view>
#include <variant>

namespace state {

template <typename Context> class AutoOpened;

template <typename Context> class AutoClosed;

template <typename Context> class Opened;

template <typename Context> class Closed;

template <typename Context>
using TState = std::variant<AutoOpened<Context>,
                            AutoClosed<Context>,
                            Opened<Context>,
                            Closed<Context>>;

template <typename Context> using MaybeState = std::optional<TState<Context>>;

template <typename Context> class BaseState {
public:
    explicit BaseState(std::reference_wrapper<Context> context_)
        : context{context_}
    {
    }

    template <typename EventT> auto process(EventT) -> MaybeState<Context>
    {
        return MaybeState<Context>{};
    }

protected:
    std::reference_wrapper<Context> context;
};

template <typename Context> class AutoOpened : public BaseState<Context> {
public:
    using BaseState<Context>::context;
    using BaseState<Context>::process;

    AutoOpened(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().open();
        context.get().get_gate().unlock();
        context.get().set_status("automatic");
    }

    auto get_state() const -> States { return States::AutoOpened; }

    auto process(PersonLeftRange) -> MaybeState<Context>
    {
        return AutoClosed<Context>{context};
    }

    auto process(ManualOpenSet) -> MaybeState<Context>
    {
        return Opened<Context>{context};
    }
};

template <typename Context> class AutoClosed : public BaseState<Context> {
public:
    using BaseState<Context>::context;
    using BaseState<Context>::process;

    AutoClosed(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().close();
        context.get().get_gate().unlock();
        context.get().set_status(std::string{"automatic"});
    }

    auto get_state() const -> States { return States::AutoClosed; }

    auto process(PersonInRange event) -> MaybeState<Context>
    {
        if (event.distance <= context.get().get_threshold()) {
            return AutoOpened<Context>{context};
        }
        return MaybeState<Context>{};
    }

    auto process(ManualCloseSet) -> MaybeState<Context>
    {
        return Closed<Context>{context};
    }

    auto process(ManualOpenSet) -> MaybeState<Context>
    {
        return Opened<Context>{context};
    }
};

template <typename Context> class Opened : public BaseState<Context> {
public:
    using BaseState<Context>::context;
    using BaseState<Context>::process;

    Opened(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().open();
        context.get().get_gate().lock();
        context.get().set_status("manual");
    }

    auto get_state() const -> States { return States::Opened; }

    auto process(ManualCloseSet) -> MaybeState<Context>
    {
        return Closed<Context>{context};
    }

    auto process(AutoModeEngaged) -> MaybeState<Context>
    {
        return AutoOpened<Context>{context};
    }
};

template <typename Context> class Closed : public BaseState<Context> {
public:
    using BaseState<Context>::context;
    using BaseState<Context>::process;

    Closed(std::reference_wrapper<Context> context_)
        : BaseState<Context>{context_}
    {
        context.get().get_gate().close();
        context.get().get_gate().lock();
        context.get().set_status("manual");
    }

    auto get_state() const -> States { return States::Closed; }

    auto process(ManualOpenSet) -> MaybeState<Context>
    {
        return Opened<Context>{context};
    }

    auto process(AutoModeEngaged) -> MaybeState<Context>
    {
        return AutoClosed<Context>{context};
    }
};

} // namespace state

namespace state_transitions {

class GateController;

using AutoOpened = state::AutoOpened<GateController>;
using AutoClosed = state::AutoClosed<GateController>;
using Opened = state::Opened<GateController>;
using Closed = state::Closed<GateController>;

class GateController {
public:
    explicit GateController(int threshold_distance_)
        : threshold_distance{threshold_distance_}
        , fsm{AutoClosed{std::ref(*this)}}
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
    patterns::FSMStateTransitions<AutoOpened, AutoClosed, Opened, Closed> fsm;
};

} // namespace state_transitions

class StatePatternFixture : public ::testing::Test {
public:
    state_transitions::GateController fsm{3};
};

TEST_F(StatePatternFixture, initial_state)
{
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(StatePatternFixture,
       autoopen_gate_should_not_react_when_range_greater_than_treshold)
{
    fsm.process(PersonInRange{4});

    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(StatePatternFixture, autoclosed_transitions_to_autoopen)
{
    fsm.process(PersonInRange{3});

    EXPECT_EQ(States::AutoOpened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(StatePatternFixture, autoopen_transitions_to_autoclose)
{
    fsm.process(PersonInRange{3});

    fsm.process(PersonLeftRange{});

    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(StatePatternFixture, autoclosed_transitions_to_closed)
{
    fsm.process(ManualCloseSet{});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, closed_state_ignores_detection_event)
{
    fsm.process(ManualCloseSet{});

    fsm.process(PersonInRange{3});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, autoclosed_transitions_to_opened)
{
    fsm.process(ManualOpenSet{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, autoopen_transitions_to_opened)
{
    fsm.process(PersonInRange{3});

    fsm.process(ManualOpenSet{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, opened_ignores_person_left_range_event)
{
    fsm.process(ManualOpenSet{});

    fsm.process(PersonLeftRange{});

    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, opened_transitions_to_closed)
{
    fsm.process(ManualOpenSet{});

    fsm.process(ManualCloseSet{});

    EXPECT_EQ(States::Closed, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, closed_transitions_to_opened)
{
    fsm.process(ManualCloseSet{});

    fsm.process(ManualOpenSet{});
    EXPECT_EQ(States::Opened, fsm.get_state());
    EXPECT_TRUE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("manual", fsm.get_status());
}

TEST_F(StatePatternFixture, opened_transitions_to_autoopened)
{
    fsm.process(ManualOpenSet{});

    fsm.process(AutoModeEngaged{});

    EXPECT_EQ(States::AutoOpened, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_TRUE(fsm.get_gate().is_open());
    EXPECT_EQ("automatic", fsm.get_status());
}

TEST_F(StatePatternFixture, closed_transitions_to_autoclosed)
{
    fsm.process(ManualCloseSet{});

    fsm.process(AutoModeEngaged{});
    EXPECT_EQ(States::AutoClosed, fsm.get_state());
    EXPECT_FALSE(fsm.get_gate().is_open());
    EXPECT_FALSE(fsm.get_gate().is_locked());
    EXPECT_EQ("automatic", fsm.get_status());
}

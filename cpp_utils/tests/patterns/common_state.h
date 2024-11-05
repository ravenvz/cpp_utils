#ifndef COMMON_STATE_H_MYVKMNOE
#define COMMON_STATE_H_MYVKMNOE

enum class States { AutoOpened, AutoClosed, Opened, Closed };

// events

struct PersonInRange {
    int distance;
};

struct PersonLeftRange { };

struct AutoModeEngaged { };

struct AutoModeSet { };

struct ManualCloseSet { };

struct ManualOpenSet { };

class Gate {
public:
    auto is_open() const noexcept -> bool { return opened; }

    auto open() noexcept -> void { opened = true; }

    auto close() noexcept -> void { opened = false; }

    auto is_locked() const noexcept -> bool { return locked; }

    auto lock() noexcept -> void { locked = true; }

    auto unlock() noexcept -> void { locked = false; }

private:
    bool opened{false};
    bool locked{false};
};

#endif /* end of include guard: COMMON_STATE_H_MYVKMNOE */

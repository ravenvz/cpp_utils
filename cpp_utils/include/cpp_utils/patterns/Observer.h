#ifndef OBSERVER_H_WEU1WSZA
#define OBSERVER_H_WEU1WSZA

#include <concepts>
#include <functional>
#include <map>
#include <type_traits>
#include <utility>

namespace patterns {

template <typename F, typename Subject>
concept ObserverCallback =
    std::invocable<F, Subject&> &&
    std::same_as<std::invoke_result_t<F, Subject&>, void>;

/* Classic GOF Observer pattern is not that hard to implement. Other variations 
 * probably have too many specific trade-offs depending on use case to have library implementations.
 *
 * This implementation strives to provide generic enough approach leveraging modern language features.
 *
 * It doesn't use virtual functions but relies on type erasure.
 *
 * Observer can be any callable including regular functions, lambda expressions, function objects,
 * and member functions, provided they are constructible into the move_only_function's specified signature.
 *
 * Attachment to Subject returns Connection object, that is able to disconnect
 * itself on destruction with RAII, preventing possible dangling
 * observer issues.
 *
 * It handles possible detachment of observers during notification (at expense of observer attachment and detachment operations
 * having O(log n) complexity).
 *
 * Observers are notified in order of registration.
 */
template <typename T> class Subject {
public:
    static_assert(std::is_class_v<T>, "Subject type must be a class");

    using Callback = std::move_only_function<void(T&)>;

    class [[nodiscard]] Connection {
        friend class Subject;

        Connection(Subject* subject_, int id_) noexcept
            : subject{subject_}
            , id{id_}
        {
        }

    public:
        Connection() = default;

        ~Connection() { disconnect(); }

        Connection(Connection&& other) noexcept
            : subject{std::exchange(other.subject, nullptr)}
            , id{std::exchange(other.id, -1)}
        {
        }

        auto operator=(Connection&& other) noexcept -> Connection&
        {
            if (this != &other) {
                disconnect();
                subject = std::exchange(other.subject, nullptr);
                id = std::exchange(other.id, -1);
            }
            return *this;
        }

        Connection(const Connection&) = delete;
        Connection& operator=(const Connection&) = delete;

        explicit operator bool() const noexcept { return subject != nullptr; }

        auto disconnect() noexcept -> void
        {
            if (subject) {
                subject->detach(id);
                subject = nullptr;
                id = -1;
            }
        }

    private:
        Subject* subject{nullptr};
        int id{-1};
    };

    template <typename F>
        requires ObserverCallback<F, T>
    [[nodiscard]] auto attach(F&& cb) -> Connection
    {
        const int id = next_id++;
        callbacks.try_emplace(id, std::forward<F>(cb));
        return Connection{this, id};
    }

protected:
    auto notify() -> void
    {
        if (callbacks.empty()) {
            return;
        }

        T& self = static_cast<T&>(*this);

        for (auto it = callbacks.begin(); it != callbacks.end();) {
            auto current = it++;

            if (current->second) {
                current->second(self);
            }
        }
    }

private:
    std::map<int, Callback> callbacks;
    int next_id = 0;

    auto detach(int id) noexcept -> void { callbacks.erase(id); }
};

} // namespace patterns

#endif /* end of include guard: OBSERVER_H_WEU1WSZA */

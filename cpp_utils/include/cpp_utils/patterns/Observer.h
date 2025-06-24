#ifndef OBSERVER_H_WEU1WSZA
#define OBSERVER_H_WEU1WSZA

#include <unordered_set>

namespace patterns {

class Subject;

class Observer {
public:
    virtual ~Observer() = default;

    virtual auto notify(Subject* subject) -> void = 0;
};

class Subject {
public:
    virtual ~Subject() = default;

    auto attach(Observer* observer) -> void { observers.insert(observer); }

    auto detach(Observer* observer) -> void { observers.erase(observer); }

    auto notify_observers() -> void
    {
        for (auto* observer : observers) {

            if (observer == nullptr) {
                observers.erase(observer);
                continue;
            }
            observer->notify(this);
        }
    }

private:
    std::unordered_set<Observer*> observers;

    auto not_null(Observer* observer) const noexcept -> bool
    {
        return observer != nullptr;
    }
};

} // namespace patterns

#endif /* end of include guard: OBSERVER_H_WEU1WSZA */

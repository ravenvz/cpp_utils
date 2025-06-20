#ifndef MEDIATOR_H_LYO4I2X7
#define MEDIATOR_H_LYO4I2X7

#include <algorithm>
#include <functional>
#include <ranges>
#include <unordered_set>

namespace patterns {

template <typename ColleagueT> class Mediator {
public:
    virtual ~Mediator() = default;

    void addColleague(ColleagueT* colleague) { colleagues.insert(colleague); }

    void removeColleague(ColleagueT* colleague) { colleagues.erase(colleague); }

protected:
    template <typename Func> void notifyAll(Func func)
    {
        auto notNull = [](ColleagueT* col) -> bool { return col; };
        std::ranges::for_each(colleagues | std::views::filter(notNull), func);
    }

    template <typename Func> void mediate(ColleagueT* caller, Func func)
    {
        auto notCaller = [&](ColleagueT* col) { return col && col != caller; };
        std::ranges::for_each(colleagues | std::views::filter(notCaller), func);
    }

private:
    std::unordered_set<ColleagueT*> colleagues;
};

} // namespace patterns

#endif /* end of include guard: MEDIATOR_H_LYO4I2X7 */

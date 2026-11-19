#pragma once
#include <algorithm>
namespace twinleap
{
struct Time
{
    static constexpr float dt = 1.f / 120;
    float accumulator{};
    void add(float elapsed)
    {
        accumulator += std::clamp(elapsed, 0.f, 0.25f);
    }
    bool ready() const
    {
        return accumulator >= dt;
    }
    void consume()
    {
        accumulator -= dt;
    }
    void reset()
    {
        accumulator = 0;
    }
};
} // namespace twinleap

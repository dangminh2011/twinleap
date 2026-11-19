#pragma once
#include <algorithm>
namespace twinleap
{
struct Vec2
{
    float x{}, y{};
};
struct Rect
{
    float x{}, y{}, w{}, h{};
};
inline bool overlaps(Rect a, Rect b)
{
    return a.x < b.x + b.w && a.x + a.w > b.x && a.y < b.y + b.h && a.y + a.h > b.y;
}
inline float approach(float a, float b, float d)
{
    return a < b ? std::min(a + d, b) : std::max(a - d, b);
}
} // namespace twinleap

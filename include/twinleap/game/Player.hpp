#pragma once
#include "twinleap/core/Math.hpp"
namespace twinleap
{
struct PlayerInput
{
    float moveX{};
    bool jumpPressed{}, jumpHeld{}, jumpReleased{};
};
struct Player
{
    Vec2 pos{}, vel{}, previous{};
    bool onGround{};
    float coyoteTimer{}, jumpBufferTimer{}, respawnTimer{};
    int facing{1};
    enum class Status
    {
        Alive,
        Dead,
        AtExit
    } status{Status::Alive};
    Rect bounds() const;
};
} // namespace twinleap

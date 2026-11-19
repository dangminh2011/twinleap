#pragma once
#include "twinleap/game/Player.hpp"
#include <array>
namespace twinleap
{
class Camera
{
  public:
    Vec2 position{};
    void update(const std::array<Player, 2> &players, int width, int height, float dt);
};
} // namespace twinleap

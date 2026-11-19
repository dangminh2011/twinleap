#pragma once
#include "twinleap/game/Level.hpp"
#include "twinleap/game/Player.hpp"
#include <array>
namespace twinleap
{
bool updateDoors(Level &level, const std::array<Player, 2> &players, float dt);
}

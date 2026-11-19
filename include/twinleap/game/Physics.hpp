#pragma once
#include "twinleap/game/Level.hpp"
#include "twinleap/game/Player.hpp"
namespace twinleap::Physics
{
constexpr float GRAVITY = 1100, MAX_FALL_SPEED = 420, RUN_SPEED = 110, GROUND_ACCEL = 900, GROUND_FRICTION = 1000,
                AIR_ACCEL = 600, AIR_FRICTION = 200, JUMP_VELOCITY = -340, JUMP_CUT_MULTIPLIER = 0.45f,
                COYOTE_TIME = 0.09f, JUMP_BUFFER_TIME = 0.10f, RESPAWN_DELAY = 1;
void step(Player &p, const PlayerInput &in, const Level &level, const Player *other, float dt);
} // namespace twinleap::Physics

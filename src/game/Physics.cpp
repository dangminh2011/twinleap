#include "twinleap/game/Physics.hpp"
#include <cmath>
namespace twinleap::Physics
{
static bool solid(Rect r, const Level &l)
{
    for (int y = int(std::floor(r.y / 16)); y <= int(std::floor((r.y + r.h - 0.01f) / 16)); ++y)
        for (int x = int(std::floor(r.x / 16)); x <= int(std::floor((r.x + r.w - 0.01f) / 16)); ++x)
            if (l.isSolidAt(x, y))
                return true;
    return false;
}
void step(Player &p, const PlayerInput &in, const Level &l, const Player *other, float dt)
{
    if (p.status != Player::Status::Alive)
        return;
    p.previous = p.pos;
    if (other && other->status == Player::Status::Alive && p.onGround &&
        std::abs(p.pos.y + 14 - other->previous.y) < 1.5f && p.pos.x + 12 > other->previous.x &&
        p.pos.x < other->previous.x + 12)
    {
        float dx = other->pos.x - other->previous.x;
        Rect carried = p.bounds();
        carried.x += dx;
        if (!solid(carried, l))
            p.pos.x += dx;
        if (other->vel.y < 0)
            p.vel.y = std::min(p.vel.y, JUMP_VELOCITY * 0.9f);
    }
    p.coyoteTimer = p.onGround ? COYOTE_TIME : std::max(0.f, p.coyoteTimer - dt);
    p.jumpBufferTimer = in.jumpPressed ? JUMP_BUFFER_TIME : std::max(0.f, p.jumpBufferTimer - dt);
    if (p.jumpBufferTimer > 0 && p.coyoteTimer > 0)
    {
        p.vel.y = JUMP_VELOCITY;
        p.onGround = false;
        p.coyoteTimer = 0;
        p.jumpBufferTimer = 0;
    }
    if (in.jumpReleased && p.vel.y < 0)
        p.vel.y *= JUMP_CUT_MULTIPLIER;
    if (in.moveX != 0)
        p.facing = in.moveX > 0 ? 1 : -1;
    p.vel.x = approach(
        p.vel.x, in.moveX * RUN_SPEED,
        (in.moveX != 0 ? (p.onGround ? GROUND_ACCEL : AIR_ACCEL) : (p.onGround ? GROUND_FRICTION : AIR_FRICTION)) * dt);
    p.vel.y = std::min(MAX_FALL_SPEED, p.vel.y + GRAVITY * dt);
    float dx = p.vel.x * dt;
    p.pos.x += dx;
    if (solid(p.bounds(), l))
    {
        p.pos.x -= dx;
        float sign = dx > 0 ? 1.f : -1.f;
        for (float moved = 0; moved < std::abs(dx); moved += 0.25f)
        {
            p.pos.x += sign * 0.25f;
            if (solid(p.bounds(), l))
            {
                p.pos.x -= sign * 0.25f;
                break;
            }
        }
        p.vel.x = 0;
    }
    float dy = p.vel.y * dt;
    float oldFoot = p.pos.y + 14;
    p.pos.y += dy;
    p.onGround = false;
    if (solid(p.bounds(), l))
    {
        p.pos.y -= dy;
        float sign = dy > 0 ? 1.f : -1.f;
        for (float moved = 0; moved < std::abs(dy); moved += 0.25f)
        {
            p.pos.y += sign * 0.25f;
            if (solid(p.bounds(), l))
            {
                p.pos.y -= sign * 0.25f;
                break;
            }
        }
        p.onGround = dy >= 0;
        p.vel.y = 0;
    }
    if (dy >= 0)
    {
        for (int y = int(oldFoot / 16); y <= int((p.pos.y + 15) / 16); ++y)
            for (int x = int(p.pos.x / 16); x <= int((p.pos.x + 11.99f) / 16); ++x)
                if (l.tileAt(x, y) == '=' && oldFoot <= float(y * 16) + 0.1f && p.pos.y + 14 >= float(y * 16))
                {
                    p.pos.y = float(y * 16) - 14;
                    p.vel.y = 0;
                    p.onGround = true;
                }
        if (other && other->status != Player::Status::Dead && p.pos.x + 12 > other->pos.x &&
            p.pos.x < other->pos.x + 12 && oldFoot <= other->previous.y + 1 && p.pos.y + 14 >= other->pos.y)
        {
            p.pos.y = other->pos.y - 14;
            p.vel.y = 0;
            p.onGround = true;
        }
    }
    Rect probe = p.bounds();
    probe.y += 1;
    if (p.vel.y >= 0 && solid(probe, l))
        p.onGround = true;
}
} // namespace twinleap::Physics

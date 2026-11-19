#include "twinleap/game/Entities.hpp"
namespace twinleap
{
bool updateDoors(Level &level, const std::array<Player, 2> &players, float dt)
{
    bool changed = false;
    bool held[2]{};
    for (auto &o : level.objects)
        if (o.kind == 'a' || o.kind == 'b')
        {
            o.active = false;
            Rect top{o.box.x, o.box.y + 12, 16, 5};
            for (const auto &p : players)
                if (p.status != Player::Status::Dead && p.onGround && overlaps({p.pos.x, p.pos.y + 13, 12, 2}, top))
                    o.active = true;
            held[o.kind - 'a'] |= o.active;
        }
    for (auto &o : level.objects)
        if (o.kind == 'A' || o.kind == 'B')
        {
            bool old = o.active;
            if (held[o.kind - 'A'])
                o.timer = 0.2f;
            else
                o.timer = std::max(0.f, o.timer - dt);
            bool occupied = false;
            for (const auto &p : players)
                occupied |= p.status != Player::Status::Dead && overlaps(p.bounds(), o.box);
            o.active = o.timer > 0 || (o.active && occupied);
            changed |= old != o.active;
        }
    return changed;
}
} // namespace twinleap

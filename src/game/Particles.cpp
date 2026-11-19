#include "twinleap/game/Particles.hpp"
namespace twinleap
{
void Particles::burst(Vec2 pos, SDL_Color color, int count)
{
    for (auto &p : pool)
        if (p.life <= 0 && count-- > 0)
        {
            random = random * 1664525u + 1013904223u;
            p = {pos, {float(int(random % 121) - 60), float(-int((random >> 8) % 120))}, 0.6f, color};
        }
}
void Particles::update(float dt)
{
    for (auto &p : pool)
        if (p.life > 0)
        {
            p.life -= dt;
            p.vel.y += 200 * dt;
            p.pos.x += p.vel.x * dt;
            p.pos.y += p.vel.y * dt;
        }
}
void Particles::render(Renderer &r) const
{
    for (const auto &p : pool)
        if (p.life > 0)
            r.rect({p.pos.x, p.pos.y, 2, 2}, p.color);
}
} // namespace twinleap
